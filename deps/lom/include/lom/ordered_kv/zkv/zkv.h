#pragma once

#include "../_util.h"

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
        */
        bool create_if_missing = false;

        //提供给后台任务的错误处理回调，若指定，则快照落盘等后台任务如果出现错误，会调用这个函数
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
