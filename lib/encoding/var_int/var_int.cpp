#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_encode(::std::tuple<::ppgo::tp_string> &ret, ::ppgo::tp_int n)
{
    std::get<0>(ret) = ::lom::var_int::Encode(n);
    return nullptr;
}

::ppgo::Exc::Ptr func_decode_impl(
    ::std::tuple<::ppgo::tp_int, ::ppgo::tp_int> &ret, ::ppgo::tp_string s, ::ppgo::tp_int begin)
{
    const char *p = s.Data() + begin;
    ssize_t sz = s.Len() - begin;
    ssize_t old_sz = sz;
    int64_t n;
    if (::lom::var_int::Decode(p, sz, n))
    {
        std::get<0>(ret) = n;
        Assert(old_sz > sz);
        std::get<1>(ret) = old_sz - sz;
    }
    else
    {
        std::get<1>(ret) = 0;
    }
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
