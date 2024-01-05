#include "internal.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

void DBImpl::Snapshot::DBGet(
    const Str &k, std::function<void (std::function<bool (StrSlice &)> const &, StrSlice)> const &f) const
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
        auto iter = v_ptr->NewForwardIterator();
        for (;;)
        {
            StrSlice iter_k, iter_v;
            if (!iter(iter_k))
            {
                break;
            }
            Assert(iter(iter_v));

            if (ks == iter_k)
            {
                f(iter, iter_v);
                return;
            }
        }
    }

    //not found
    f(nullptr, StrSlice());
}

LOM_ERR DBImpl::Snapshot::DBGet(const Str &k, std::function<void (const StrSlice *)> const &f) const
{
    DBGet(k, [f] (std::function<bool (StrSlice &)> const &iter, StrSlice v) {
        return iter ? f(&v) : f(nullptr);
    });
    return nullptr;
}

LOM_ERR DBImpl::Snapshot::DBGet(const Str &k, std::function<StrSlice ()> &v) const
{
    DBGet(k, [&v] (std::function<bool (StrSlice &)> const &iter, StrSlice iter_v) {
        if (iter)
        {
            v = [iter, iter_v] () -> StrSlice {
                static_cast<void>(iter);    //just a holder
                return iter_v;
            };
        }
        else
        {
            v = nullptr;
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
