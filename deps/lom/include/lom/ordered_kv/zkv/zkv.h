#pragma once

#include "../_util.h"

#include "_lite.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

class DB : public DBBase
{
public:

    typedef std::shared_ptr<DB> Ptr;

    typedef ::lom::immut::AVL<Str, ::lom::immut::ZList> ZMap;

    virtual LOM_ERR Write(const WriteBatch &wb, std::function<void ()> const &commit_hook) = 0;
    virtual LOM_ERR Write(const WriteBatch &wb) override
    {
        return Write(wb, nullptr);
    }

    virtual Snapshot::Ptr NewSnapshot(std::function<void ()> const &new_snapshot_hook) = 0;
    virtual Snapshot::Ptr NewSnapshot() override
    {
        return NewSnapshot(nullptr);
    }

    //返回粗略的空间耗费值
    virtual ssize_t SpaceCost() = 0;

    //获取底层的数据
    virtual ZMap RawZMap() = 0;

    /*
    在指定`path`时为持久化存储模式，会在对应目录下以数据Dump+WAL的形式进行落盘
    若`path`为空指针则为纯内存模式，不会进行IO操作
    */
    struct Options
    {
        /*
        为false时打开已有库，库不存在则失败
        为true时尝试打开，若库不存在则创建一个新的
            创建过程中，目录创建是增量的，因此允许通过软连接预先安排不同目录的存储布局
        持久化模式下有效
        */
        bool create_if_missing = false;

        /*
        提供给后台任务的错误处理回调，若指定，则快照落盘等后台任务如果出现错误，会调用这个函数
        持久化模式下有效
        */
        std::function<void (LOM_ERR)> handle_bg_err;
    };
    static LOM_ERR Open(const char *path, Ptr &db, Options opts);
    static LOM_ERR Open(const char *path, Ptr &db)
    {
        return Open(path, db, Options());
    }
    static LOM_ERR Open(Ptr &db, Options opts)
    {
        return Open(nullptr, db, opts);
    }
    static LOM_ERR Open(Ptr &db)
    {
        return Open(db, Options());
    }
};

}

}

}
