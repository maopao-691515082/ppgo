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

    virtual ~DB()
    {
    }

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
    virtual Err::Ptr Write(
        const WriteBatch &wb, ssize_t *add_count = nullptr, ssize_t *del_count = nullptr) = 0;
};

}

}
