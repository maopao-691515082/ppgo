#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_copy_from(
    ::std::tuple<::ppgo::tp_int> &ret,
    ::ppgo::VecView<::ppgo::tp_byte> b, ::ppgo::VecView<::ppgo::tp_byte> from_b)
{
    if (!(b.Valid() && from_b.Valid()))
    {
        ::ppgo::Exc::Sprintf("invalid view");
    }
    auto copy_len = std::min(b.Len(), from_b.Len());
    memcpy(b.GetElemPtr(0), from_b.GetElemPtr(0), copy_len);
    std::get<0>(ret) = copy_len;
    return nullptr;
}

::ppgo::Exc::Ptr func_index_byte(
    ::std::tuple<::ppgo::tp_int> &ret, ::ppgo::VecView<::ppgo::tp_byte> b, ::ppgo::tp_byte c)
{
    if (!b.Valid())
    {
        ::ppgo::Exc::Sprintf("invalid view");
    }
    auto p = b.GetElemPtr(0);
    auto len = b.Len();
    for (ssize_t i = 0; i < len; ++ i)
    {
        if (p[i] == c)
        {
            std::get<0>(ret) = i;
            return nullptr;
        }
    }
    std::get<0>(ret) = -1;
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
