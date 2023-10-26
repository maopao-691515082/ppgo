#include "internal.h"

namespace lom
{

namespace ordered_kv
{

namespace ckv
{

static const Str
    kMeta_NextDataFileIdKey = "NFID",
    kMeta_DataFileKeyPrefix = "F_",
    kMeta_SegKeyPrefix = "S_",
    kDataFileNamePrefix = "DATA-";

class DefaultMetaDB : public MetaDB
{
    ::lom::ordered_kv::zkv::DB::Ptr zkv_;

public:

    LOM_ERR Init(const char *path, Options opts)
    {
        return ::lom::ordered_kv::zkv::DB::Open(
            path,
            zkv_,
            ::lom::ordered_kv::zkv::DB::Options{
                .create_if_missing  = opts.create_if_missing,
                .handle_bg_err      = opts.handle_bg_err,
            }
        );
    }

    virtual WriteBatchBase::Ptr NewWriteBatch() override
    {
        return WriteBatchBase::Ptr(new WriteBatch());
    }

    virtual LOM_ERR Write(const WriteBatchBase &wb, std::function<void ()> commit_hook) override
    {
        return zkv_->Write(dynamic_cast<const WriteBatch &>(wb), commit_hook);
    }

    virtual Snapshot::Ptr NewSnapshot(std::function<void ()> new_snapshot_hook) override
    {
        return zkv_->NewSnapshot(new_snapshot_hook);
    }
};

static void SetNextDataFileId(WriteBatchBase &wb, ssize_t id)
{
    wb.Put(kMeta_NextDataFileIdKey, ::lom::var_int::Encode(id));
}

static LOM_ERR GetNextDataFileId(const ::lom::ordered_kv::Snapshot &snapshot, ssize_t &nid) const
{
    Str v;
    bool ok;
    LOM_RET_ON_ERR(snapshot.Get(kMeta_NextDataFileIdKey, v, ok));
    if (!ok)
    {
        LOM_RET_ERR("next-file-id lost");
    }

    auto p = v.Data();
    auto sz = v.Len();
    int64_t id;
    if (!(::lom::var_int::Decode(p, sz, id) && sz == 0 && id > 0))
    {
        LOM_RET_ERR("invalid next-file-id");
    }

    nid = id;
    return nullptr;
}

static bool ParseDataFileInfo(StrSlice info, ssize_t &seg_count, ssize_t &freed_seg_count)
{
    auto p = info.Data();
    auto sz = info.Len();
    int64_t sc, fsc;
    bool ok =
        ::lom::var_int::Decode(p, sz, sc) && ::lom::var_int::Decode(p, sz, fsc) &&
        sz == 0 && sc >= 0 && fsc >= 0;
    if (ok)
    {
        seg_count = sc;
        freed_seg_count = fsc;
    }
    return ok;
}

LOM_ERR DBImpl::Init(const char *path, Options opts)
{
    core_ = std::make_shared<Core>();

    core_->path = ::lom::os::Path(path_str).Str();

    auto open_meta_db = opts.open_meta_db;
    if (!open_meta_db)
    {
        open_meta_db =
            [] (const char *meta_db_path, MetaDB::Ptr &meta_db, MetaDB::Options meta_db_opts) -> LOM_ERR {
                auto mdb = std::make_shared<DefaultMetaDB>();
                LOM_RET_ON_ERR(mdb->Init(meta_db_path, meta_db_opts));
                meta_db = static_pointer_cast<MetaDB::Ptr>(mdb);
                return nullptr;
            }
        ;
    }

    if (opts.create_if_missing)
    {
        LOM_RET_ON_ERR(::lom::os::MakeDirs(path_.CStr()));
    }

    auto meta_db_path = path_.Concat("/META");
    LOM_RET_ON_ERR(open_meta_db(
        meta_db_path.CStr(),
        core_->meta_db,
        MetaDB::Options{
            .create_if_missing  = opts.create_if_missing,
            .handle_bg_err      = opts.handle_bg_err,
        }
    ));

    {
        auto snapshot = core_->meta_db->NewSnapshot();
        Str serial_k = "SERIAL";
        bool ok;
        LOM_RET_ON_ERR(snapshot->Get(serial_k, core_->serial, ok));
        if (ok)
        {
            ssize_t nid;
            LOM_RET_ON_ERR(GetNextDataFileId(*snapshot, nid));

            auto iter = snapshot->NewIterator();
            for (iter->Seek(kMeta_DataFileKeyPrefix); iter->Valid(); iter->Next())
            {
                auto k = iter->Key();
                if (!k.HasPrefix(kMeta_DataFileKeyPrefix))
                {
                    break;
                }

                ssize_t id;
                if (!(k.Slice(kMeta_DataFileKeyPrefix.Len()).ParseSSize(id) && id > 0))
                {
                    LOM_RET_ERR("invalid meta, data file key %s", k.Repr().CStr());
                }
                if (id >= nid)
                {
                    LOM_RET_ERR("invalid meta, file id %zd >= next file id %zd", id, nid);
                }

                auto v = iter->Value();
                ssize_t seg_count, freed_seg_count;
                if (!ParseDataFileInfo(v, seg_count, freed_seg_count))
                {
                    LOM_RET_ERR("invalid meta, invalid data file info of id %zd", id);
                }

                auto dfp = ::lom::Sprintf("%s/%s%zd", core_->path.CStr(), kDataFilePrefix.CStr(), id);
                ::lom::os::File::Ptr df;
                LOM_RET_ON_ERR(::lom::os::File::Open(dfp.CStr(), df, "r"));

                if (core_->data_files.HasKey(id))
                {
                    LOM_RET_ERR("invalid meta, duplicate data file id %zd", id);
                }
                core_->data_files =
                    core_->data_files.Set(
                        id, DataFile::Ptr(new DataFile(id, df, seg_count, freed_seg_count)));
            }
            LOM_RET_ON_ERR(iter->Err());
        }
        else
        {
            auto iter = snapshot->NewIterator();
            LOM_RET_ON_ERR(iter->Err());
            if (!iter->IsRightBorder())
            {
                LOM_RET_ERR("invalid meta db `%s`: non-empty without serial", meta_db_path.CStr());
            }

            core_->serial = ::lom::Sprintf(
                "CKV-%zu-%zu",
                static_cast<size_t>(::lom::NowUS()),
                static_cast<size_t>(::lom::RandU64()));
            auto wb = core_->meta_db->NewWriteBatch();
            wb->Set(serial_k, core_->serial);
            SetNextFileId(*wb, 1);
            LOM_RET_ON_ERR(core_->meta_db->Write(*wb));
        }
    }

    //todo
}

DBImpl::~DBImpl()
{
    //todo
}

LOM_ERR DBImpl::Write(const WriteBatch &wb, std::function<void ()> commit_hook)
{
    auto const &wb_ops = wb.RawOps();
    if (wb_ops.empty())
    {
        if (commit_hook)
        {
            core_->meta_db->Write(WriteBatch(), commit_hook);
        }
        return nullptr;
    }

    //todo
}

::lom::ordered_kv::Snapshot::Ptr DBImpl::NewSnapshot(std::function<void ()> new_snapshot_hook)
{
    //todo
}

LOM_ERR DB::Open(const char *path, Ptr &db, Options opts)
{
    auto new_db = std::make_shared<DBImpl>();
    LOM_RET_ON_ERR(new_db->Init(path, opts));
    db = std::static_pointer_cast<DB>(new_db);
    return nullptr;
}

}

}

}
