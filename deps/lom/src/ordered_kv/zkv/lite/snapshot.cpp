#include "internal.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

namespace experimental
{

static bool ZMGet(const DBLite::ZMap &zm, const Str &k, StrSlice &v)
{
    ssize_t idx;
    auto gs_ptr = zm.Get(k, &idx);
    if (!gs_ptr && idx > 0)
    {
        gs_ptr = zm.GetByIdx(idx - 1).second;
    }
    if (gs_ptr)
    {
        auto ks = k.Slice();
        for (ssize_t i = 0; i < gs_ptr->Len(); i += 2)
        {
            if (ks == gs_ptr->At(i).Slice())
            {
                v = gs_ptr->At(i + 1).Slice();
                return true;
            }
        }
    }

    return false;
}

LOM_ERR DBLiteImpl::Snapshot::DBGet(const Str &k, std::function<void (const StrSlice *)> const &f) const
{
    StrSlice v;
    f(ZMGet(zm_, k, v) ? &v : nullptr);
    return nullptr;;
}

LOM_ERR DBLiteImpl::Snapshot::DBGet(const Str &k, std::function<StrSlice ()> &v_getter) const
{
    StrSlice v;
    v_getter =
        ZMGet(zm_, k, v) ?
            std::function<StrSlice ()>(
                [zm = zm_, v] () -> StrSlice {
                    static_cast<void>(zm);
                    return v;
                }
            ) :
            nullptr;
    return nullptr;;
}

::lom::ordered_kv::Iterator::Ptr DBLiteImpl::Snapshot::DBNewIterator() const
{
    return ::lom::ordered_kv::Iterator::Ptr(new Iterator(zm_));
}

}

}

}

}
