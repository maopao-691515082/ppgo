#pragma once

#include "../_internal.h"

#include "../str.h"
#include "../err.h"

namespace lom
{

namespace ckv
{

//索引库接口，如果需要指定索引库的实现，则继承实现这个接口（当然也包括内部的`WriteBatch`等接口）
class IndexDB
{
public:

    typedef std::shared_ptr<IndexDB> Ptr;

    virtual ~IndexDB() = default;

    class WriteBatch
    {
    public:

        typedef std::shared_ptr<WriteBatch> Ptr;

        virtual ~WriteBatch() = default;

        virtual void Set(const Str &k, const Str &v) = 0;
        virtual void Del(const Str &k) = 0;
    };

    virtual WriteBatch::Ptr NewWriteBatch() = 0;
    virtual ::lom::Err::Ptr Write(const WriteBatch::Ptr &wb) = 0;

    class Snapshot
    {
    public:

        typedef std::shared_ptr<Snapshot> Ptr;

        virtual ~Snapshot() = default;

        virtual ::lom::Err::Ptr Get(const Str &k, Str &v) const = 0;

        /*
        指定前缀迭代，回调函数的参数`k_suffix`表示迭代到的K中去除前缀后的后缀部分，
        例如迭代到数据库中的K是`abc`，前缀`k_prefix`指定是`ab`，则`k_suffix`在回调时就是`c`
        */
        virtual ::lom::Err::Ptr IterKVSByKeyPrefix(
            const Str &k_prefix,
            std::function<
                void (StrSlice /*k*/, StrSlice /*k_suffix*/, StrSlice /*v*/, bool & /*stop*/)
            > f
        ) const = 0;
    };
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
class DB
{
public:

    typedef std::shared_ptr<DB> Ptr;

    virtual ~DB() = default;

    //批量写操作的执行请求对象
    class WriteBatch
    {
    public:

        typedef std::map<Str /*k*/, std::pair<bool /*is_del*/, Str /*v*/>> RawOpsMap;

    private:

        RawOpsMap raw_ops_;

    public:

        const RawOpsMap &RawOps() const
        {
            return raw_ops_;
        }

        ssize_t Size() const
        {
            return static_cast<ssize_t>(raw_ops_.size());
        }

        void IterAll(std::function<void (const Str &k, const Str *v /*nullptr means to del*/)> f) const
        {
            for (const auto &p : raw_ops_)
            {
                f(p.first, p.second.first ? nullptr : &p.second.second);
            }
        }

        void Reset()
        {
            raw_ops_.clear();
        }

        void Set(const Str &k, const Str &v)
        {
            raw_ops_[k] = std::make_pair(false, v);
        }

        void Del(const Str &k)
        {
            raw_ops_[k] = std::make_pair(true, Str());
        }
    };

    //执行写操作，`add_count`和`del_count`可用于保存本次操作导致的key的新增和删除数量
    virtual ::lom::Err::Ptr Write(
        const WriteBatch &wb, ssize_t *add_count = nullptr, ssize_t *del_count = nullptr) = 0;

    //内部实现的`Snapshot`对象会引用到对应的`DB`对象，因此总是可用的，不会出现脱节失效
    class Snapshot
    {
    public:

        typedef std::shared_ptr<Snapshot> Ptr;

        virtual ~Snapshot() = default;

        //获取值并调用回调函数，`f`的参数为空指针表示没有找到
        virtual ::lom::Err::Ptr Get(
            const Str &k, std::function<void (const StrSlice * /*ptr to v*/)> f) const = 0;
        //获取值，通过`v`返回一个值的获取器，`v`返回值为空函数表示没有找到
        virtual ::lom::Err::Ptr Get(const Str &k, std::function<StrSlice ()> &v) const = 0;

        /*
        迭代所有KV对，并调用对应的回调函数`f`
        `f`通过第三个参数返回是否停止迭代（`stop`默认`false`，需要停止时`f`内只需要将其改为`true`即可）
        `start_k`可指定迭代起始点
        `skip_count`若大于0，则表示在迭代范围内跳过指定数量的KV对
        */
        typedef std::function<void (StrSlice /*k*/, StrSlice /*v*/, bool & /*stop*/)> IterCallback;
        virtual ::lom::Err::Ptr IterKVS(
            IterCallback f, const Str &start_k = "", ssize_t skip_count = 0) const = 0;

        /*
        指定前缀迭代，`f`回调中额外有一个去除了前缀的`k_suffix`，
        例如迭代到数据库中的K是`abc`，前缀`k_prefix`指定是`ab`，则`k_suffix`在回调时就是`c`
        `skip_count`含义同`IterKVS`
        */
        ::lom::Err::Ptr IterKVSByKeyPrefix(
            const Str &k_prefix,
            std::function<
                void (StrSlice /*k*/, StrSlice /*k_suffix*/, StrSlice /*v*/, bool & /*stop*/)
            > f,
            ssize_t skip_count = 0
        ) const;

        /*
        快照可以immutable地进行修改，这些修改生成的新快照等同于原数据修改后的数据视图，修改是临时的，
        并不会影响库中的数据
        通过GetModifies可以获取修改记录，常见用法是先获取一个快照，以它为基础做计算和修改，
        最终将修改记录取出并写入DB，这样模拟了单个事务内部的临时写入、原子提交和回滚流程，
        当然这并不是传统意义的数据库事务，只能看作是一种临时的工作视图功能
        */
        virtual Ptr Set(const Str &k, const Str &v) const = 0;
        virtual Ptr Del(const Str &k) const = 0;
        virtual void GetModifies(WriteBatch &wb) const = 0;
    };

    //创建一个当前DB数据的快照
    virtual Snapshot::Ptr NewSnapshot() = 0;

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

        Options() = default;
    };
    static ::lom::Err::Ptr Open(const char *path, Ptr &db, Options opts);
    static ::lom::Err::Ptr Open(const char *path, Ptr &db)
    {
        return Open(path, db, Options());
    }
};

}

}
