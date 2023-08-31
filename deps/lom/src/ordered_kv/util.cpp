#include "../internal.h"

namespace lom
{

namespace ordered_kv
{

::lom::Err::Ptr Snapshot::Get(const Str &k, std::function<void (const StrSlice * /*ptr to v*/)> f) const
{
    auto iter = wb.RawOps().find(k);
    if (iter == wb.RawOps().end())
    {
        return DBGet(k, f);
    }

    if (iter->second)
    {
        auto s = iter->second->Slice();
        f(&s);
    }
    else
    {
        f(nullptr);
    }

    return nullptr;
}

::lom::Err::Ptr Snapshot::Get(const Str &k, std::function<StrSlice ()> &v) const
{
    auto iter = wb.RawOps().find(k);
    if (iter == wb.RawOps().end())
    {
        return DBGet(k, v);
    }

    if (iter->second)
    {
        v = [s = *iter->second] () -> StrSlice {
            return s.Slice();
        };
    }
    else
    {
        v = nullptr;
    }

    return nullptr;
}

Iterator::Ptr Snapshot::NewIterator() const
{
    //todo
    return nullptr;
}

}

}
