#include "internal.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

void DBImpl::Snapshot::DBGet(
    const Str &k, std::function<void (const ::lom::immut::ZList::Iterator *)> f) const
{
    ssize_t idx;
    auto v_ptr = zm_.Get(k, &idx);
    if (v_ptr == nullptr && idx > 0)
    {
        v_ptr = zm_.GetByIdx(idx - 1).second;
    }
    if (v_ptr != nullptr)
    {
        auto ks = k.Slice();
        ::lom::immut::ZList::Iterator iter(*v_ptr);
        while (iter.Valid())
        {
            auto iter_k = iter.Get();
            iter.Next();
            Assert(iter.Valid());

            if (ks == iter_k)
            {
                f(&iter);
                return;
            }

            iter.Next();
        }
    }

    //not found
    f(nullptr);
}

::lom::Err::Ptr DBImpl::Snapshot::DBGet(const Str &k, std::function<void (const StrSlice *)> f) const
{
    DBGet(k, [f] (const ::lom::immut::ZList::Iterator *iter) {
        if (iter == nullptr)
        {
            f(nullptr);
        }
        else
        {
            auto v = iter->Get();
            f(&v);
        }
    });
    return nullptr;
}

::lom::Err::Ptr DBImpl::Snapshot::DBGet(const Str &k, std::function<StrSlice ()> &v) const
{
    DBGet(k, [&v] (const ::lom::immut::ZList::Iterator *iter_ptr) {
        if (iter_ptr == nullptr)
        {
            v = nullptr;
        }
        else
        {
            v = [iter = *iter_ptr] () -> StrSlice {
                return iter.Get();
            };
        }
    });
    return nullptr;
}

::lom::ordered_kv::Iterator::Ptr DBImpl::Snapshot::DBNewIterator() const
{
    return ::lom::ordered_kv::Iterator::Ptr(new Iterator(zm_));
}

}

}

}
