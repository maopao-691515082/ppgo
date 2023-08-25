#pragma once

#include "../_internal.h"

#include "../str.h"
#include "../err.h"

namespace lom
{

namespace ckv
{

class DB
{
public:

    typedef std::shared_ptr<DB> Ptr;

    virtual ~DB() = default;

    //批量写操作的执行请求
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

        void Put(const Str &k, const Str &v)
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
    };
};

}

}
