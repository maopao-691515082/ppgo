#pragma once

#include "../_util.h"

namespace lom
{

namespace ordered_kv
{

namespace ckv
{

/********
思路有点问题，暂时没做完：）
*********/

/*
元数据库接口
元数据库存放数据索引信息、文件管理信息等
如果需要指定实现，则继承实现这个接口（当然也包括关联的`WriteBatch`等接口）
自实现的元数据库必须保证全量K有序存储
*/
class MetaDB
{
public:

    typedef std::shared_ptr<MetaDB> Ptr;

    virtual ~MetaDB() = default;

    //允许MetaDB使用自己的批量写对象
    virtual WriteBatchBase::Ptr NewWriteBatch() = 0;

    /*
    MetaDB的实现必须支持数据的原子性提交
    而且，必须支持写操作和获取快照操作的互斥，且过程中需要能执行hook，
    这是为了保证元数据和CKV数据的一致性，避免出现一个CKV快照获取到的MetaDB快照和CKV数据快照不一致
    一般来说MetaDB的实现中可以简单弄一个锁来保证互斥
        - 写操作的`commit_hook`需要在写操作的提交过程的最后进行
        - 获取快照的`new_snapshot_hook`只需要保证和MetaDB的快照获取操作捆绑即可
    */

    virtual LOM_ERR Write(const WriteBatchBase &wb, std::function<void ()> commit_hook) = 0;

    virtual Snapshot::Ptr NewSnapshot(std::function<void ()> new_snapshot_hook) = 0;

    /*
    打开元数据库的函数接口
    注：CKV会保证在`create_if_missing`为true时，`path`的父目录先被创建，且`path`为`<CKV_PATH>/META`
    */
    struct Options
    {
        /*
        为false时打开已有库，库不存在则失败
        为true时尝试打开，若库不存在则创建一个新的
        */
        bool create_if_missing = false;

        //提供给后台任务的错误处理回调
        std::function<void (LOM_ERR)> handle_bg_err;
    };
    typedef std::function<LOM_ERR (const char *path, Ptr &db, Options opts)> OpenFunc;
};

//数据库对象
class DB : public DBBase
{
public:

    typedef std::shared_ptr<DB> Ptr;

    virtual LOM_ERR Write(const WriteBatch &wb, std::function<void ()> commit_hook) = 0;
    virtual LOM_ERR Write(const WriteBatch &wb) override
    {
        return Write(wb, nullptr);
    }

    virtual Snapshot::Ptr NewSnapshot(std::function<void ()> new_snapshot_hook) = 0;
    virtual Snapshot::Ptr NewSnapshot() override
    {
        return NewSnapshot(nullptr);
    }

    //指定目录`path`创建数据库，可指定选项
    struct Options
    {
        /*
        为false时打开已有库，库不存在则失败
        为true时尝试打开，若库不存在则创建一个新的
            创建过程中，目录创建是增量的，因此允许通过软连接预先安排不同目录的存储布局
        */
        bool create_if_missing = false;

        //提供给后台任务的错误处理回调
        std::function<void (LOM_ERR)> handle_bg_err;

        //可指定打开元数据库的函数，若不指定，则使用内部默认的实现（ZKV）
        MetaDB::OpenFunc open_meta_db;
    };
    static LOM_ERR Open(const char *path, Ptr &db, Options opts);
    static LOM_ERR Open(const char *path, Ptr &db)
    {
        return Open(path, db, Options());
    }
};

}

}

}
