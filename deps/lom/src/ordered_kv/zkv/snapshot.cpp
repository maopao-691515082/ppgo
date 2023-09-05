#include "internal.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

::lom::Err::Ptr DBImpl::Snapshot::DBGet(const Str &k, std::function<void (const StrSlice *)> f) const
{
    (void)k, (void)f;
    return ::lom::Err::Sprintf("todo");
}

::lom::Err::Ptr DBImpl::Snapshot::DBGet(const Str &k, std::function<StrSlice ()> &v) const
{
    (void)k, (void)v;
    return ::lom::Err::Sprintf("todo");
}

Iterator::Ptr DBImpl::Snapshot::DBNewIterator() const
{
    //todo
    return nullptr;
}

}

}

}
