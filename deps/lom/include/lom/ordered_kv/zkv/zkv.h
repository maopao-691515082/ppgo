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

    //数据库的状态信息
    struct Stat
    {
        //数据状态
        ssize_t key_count           = 0;    //key数量
        ssize_t data_len            = 0;    //库中数据总长

        void ApplyChange(const Stat &stat_change)
        {
            key_count           += stat_change.key_count;
            data_len            += stat_change.data_len;
        }
    };
    virtual Stat GetStat() = 0;

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
    };
    static ::lom::Err::Ptr Open(const char *path, Ptr &db, Options opts);
    static ::lom::Err::Ptr Open(const char *path, Ptr &db)
    {
        return Open(path, db, Options());
    }
    static ::lom::Err::Ptr Open(Ptr &db, Options opts)
    {
        return Open(nullptr, db, opts);
    }
    static ::lom::Err::Ptr Open(Ptr &db)
    {
        return Open(db, Options());
    }
};

}

}

}
