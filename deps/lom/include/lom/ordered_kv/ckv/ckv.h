#pragma once

#include "../_util.h"

namespace lom
{

namespace ordered_kv
{

namespace ckv
{

/*
索引库接口，如果需要指定索引库的实现，则继承实现这个接口（当然也包括关联的`WriteBatch`等接口）
自实现的索引库必须保证全量K有序存储
*/
class IndexDB
{
public:

    typedef std::shared_ptr<IndexDB> Ptr;

    virtual ~IndexDB() = default;

    virtual WriteBatchBase::Ptr NewWriteBatch() = 0;
    virtual ::lom::Err::Ptr Write(const WriteBatchBase::Ptr &wb) = 0;

    virtual Snapshot::Ptr NewSnapshot() = 0;

    //打开索引库的函数接口
    struct Options
    {
        /*
        为false时打开已有库，库不存在则失败
        为true时尝试打开，若库不存在则创建一个新的
        */
        bool create_if_missing = false;
    };
    typedef std::function<::lom::Err::Ptr (const char *path, Ptr &db, Options opts)> OpenFunc;
};

//数据库对象
class DB : public DBBase
{
public:

    typedef std::shared_ptr<DB> Ptr;

    virtual ~DB() = default;

    //数据库的状态信息
    struct Stat
    {
        //数据状态
        ssize_t key_count           = 0;    //key数量
        ssize_t data_len            = 0;    //库中数据总长
        ssize_t used_block_count    = 0;    //已使用的block数量

        void ApplyChange(const Stat &stat_change)
        {
            key_count           += stat_change.key_count;
            data_len            += stat_change.data_len;
            used_block_count    += stat_change.used_block_count;
        }

        ssize_t BlockSize() const;  //单个block的大小
    };
    virtual Stat GetStat() = 0;

    //指定目录`path`创建数据库，可指定选项
    struct Options
    {
        /*
        为false时打开已有库，库不存在则失败
        为true时尝试打开，若库不存在则创建一个新的
            创建过程中，目录创建是增量的，因此允许通过软连接预先安排不同目录的存储布局
        */
        bool create_if_missing = false;

        //可指定打开索引库的函数，若不指定，则使用内部默认的实现
        IndexDB::OpenFunc open_idx_db;
    };
    static ::lom::Err::Ptr Open(const char *path, Ptr &db, Options opts);
    static ::lom::Err::Ptr Open(const char *path, Ptr &db)
    {
        return Open(path, db, Options());
    }
};

}

}

}
