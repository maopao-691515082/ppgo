#include "internal.h"

namespace lom
{

namespace ordered_kv
{

namespace ckv
{

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

    virtual LOM_ERR Write(const WriteBatchBase &wb) override
    {
        return zkv_->Write(dynamic_cast<const WriteBatch &>(wb));
    }

    virtual Snapshot::Ptr NewSnapshot() override
    {
        return zkv_->NewSnapshot();
    }
};

LOM_ERR Init(const char *path, Options opts)
{
    path_ = ::lom::os::Path(path_str).Str();

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
        meta_db_,
        MetaDB::Options{
            .create_if_missing  = opts.create_if_missing,
            .handle_bg_err      = opts.handle_bg_err,
        }
    ));

    {
        auto snapshot = meta_db_->NewSnapshot();
        Str serial_k = "SERIAL";
        bool ok;
        LOM_RET_ON_ERR(snapshot->Get(serial_k, serial_, ok));
        if (ok)
        {
            //todo load meta data
            //files
            //segs
        }
        else
        {
            auto iter = snapshot->NewIterator();
            LOM_RET_ON_ERR(iter->Err());
            if (!iter->IsRightBorder())
            {
                LOM_RET_ERR("invalid meta db `%s`: non-empty without serial", meta_db_path.CStr());
            }

            serial_ = ::lom::Sprintf(
                "CKV-%zu-%zu",
                static_cast<size_t>(::lom::NowUS()),
                static_cast<size_t>(::lom::RandU64()));
            auto wb = meta_db_->NewWriteBatch();
            wb->Set(serial_k, serial_);
            LOM_RET_ON_ERR(meta_db_->Write(*wb));
        }
    }
}

LOM_ERR Write(const WriteBatch &wb)
{
    //todo
}

::lom::ordered_kv::Snapshot::Ptr NewSnapshot()
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
