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

void DBImpl::GCThreadMain(std::function<void (LOM_ERR)> handle_bg_err, Core::Ptr db_core)
{
    while (!db_core->stopped.load())
    {
        //获取data_files的快照（不需要meta_db的）
        DataFiles dfs;
        db_core->meta_db->NewSnapshot(
            [&] () {
                dfs = db_core->data_files;
            }
        );

        //找到需要GC的数据文件
        DataFile::Ptr df;
        if (dfs.Size() >= 16)
        {
            //最后一个文件一般来说是当前data_writer，不能被GC，需要排除掉
            for (ssize_t i = dfs.Size() - 2; i >= 0; -- i)
            {
                auto kvpp = dfs.GetByIdx(i);
                Assert(*kvpp.first == (*kvpp.second)->ID());
                if ((*kvpp.second)->NeedGC())
                {
                    df = *kvpp.second;
                    break;
                }
            }
        }
        if (!df)
        {
            //暂时没有数据文件可供GC
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        auto do_gc = [&] () -> LOM_ERR {
            auto br = df->NewPReadBufReader(0);
            ::lom::immut::ZList header_zl;
            LOM_RET_ON_ERR(::lom::immut::ZList::LoadFrom(br, header_zl));
            //打开文件的时候已经检查过header_zl是否有效，这里就不再检查了

            //渐进式执行GC，迁移选中的数据文件中的有效数据，每次一个seg
            for (;;)
            {
                if (db_core->stopped.load())
                {
                    //数据库停止，及时结束
                    return nullptr;
                }

                //todo 读取一个seg，判断是否已被释放，若没有释放，则进行迁移
            }

            //todo 成功GC了一个文件，将其删除

            return nullptr;
        };

        auto err = do_gc();
        if (err && handle_bg_err)
        {
            handle_bg_err(err);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

LOM_ERR DBImpl::Init(const char *path, Options opts)
{
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

    if (opts.handle_bg_err)
    {
        //handle_bg_err有可能被并发调用，包装成加锁同步版本
        auto handle_bg_err_sync =
            [lock = std::make_shared<std::mutex>(), handle_bg_err = opts.handle_bg_err] (LOM_ERR err) {
                std::lock_guard<std::mutex> lg(*lock);
                handle_bg_err(err);
            }
        ;
        opts.handle_bg_err = handle_bg_err_sync;
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
                    LOM_RET_ERR("invalid data file key %s", k.Repr().CStr());
                }
                if (id >= nid)
                {
                    LOM_RET_ERR("invalid meta, file id %zd >= next file id %zd", id, nid);
                }
                if (core_->data_files.HasKey(id))
                {
                    LOM_RET_ERR("invalid meta, duplicate data file id %zd", id);
                }

                auto v = iter->Value();
                ssize_t seg_count, freed_seg_count;
                if (!ParseDataFileInfo(v, seg_count, freed_seg_count))
                {
                    LOM_RET_ERR("invalid data file info of id %zd", id);
                }

                auto dfn = ::lom::Sprintf("%s%zd", kDataFilePrefix.CStr(), id);
                auto dfp = ::lom::Sprintf("%s/%s", core_->path.CStr(), dfn.CStr());
                ::lom::os::File::Ptr df;
                LOM_RET_ON_ERR(::lom::os::File::Open(dfp.CStr(), df));

                auto br = ::lom::io::BufReader::New(
                    [df] (char *buf, ssize_t sz, ssize_t &rsz) -> LOM_ERR {
                        return df->Read(buf, sz, rsz);
                    }
                );
                ::lom::immut::ZList zl;
                LOM_RET_ON_ERR(::lom::immut::ZList::LoadFrom(br, zl));
                auto head_parts = zl.Parse();
                if (!(head_parts.Len() == 2 && head_parts.At(0) == core_->serial && head_parts.At(1) == dfn))
                {
                    LOM_RET_ERR("invalid header of data file `%s`", dfp.CStr());
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

    std::thread(
        [handle_bg_err = opts.handle_bg_err, db_core = core_] () {
            GCThreadMain(handle_bg_err, db_core);
        }
    ).detach();

    return nullptr;
}

DBImpl::~DBImpl()
{
    core_->stopped.store(true);
}

LOM_ERR DBImpl::Write(const WriteBatch &wb, std::function<void ()> commit_hook)
{
    auto const &wb_ops = wb.RawOps();
    if (wb_ops.empty())
    {
        if (commit_hook)
        {
            core_->meta_db->Write(wb, commit_hook);
        }
        return nullptr;
    }

    std::lock_guard<std::mutex> wlg(write_lock_);

    //todo
}

::lom::ordered_kv::Snapshot::Ptr DBImpl::NewSnapshot(std::function<void ()> new_snapshot_hook)
{
    DataFiles dfs;
    auto meta_snapshot = core_->meta_db->NewSnapshot(
        [&] () {
            dfs = core_->data_files;
            if (new_snapshot_hook)
            {
                new_snapshot_hook();
            }
        }
    );

    return ::lom::ordered_kv::Snapshot::Ptr(new Snapshot(meta_snapshot, dfs));
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
