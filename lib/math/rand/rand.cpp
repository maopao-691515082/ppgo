#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_rand(::std::tuple<::ppgo::tp_float> &ret)
{
    ::std::get<0>(ret) = ::ppgo::tp_float{::lom::Rand()};
    return nullptr;
}

::ppgo::Exc::Ptr func_rand_n(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int n)
{
    ::ppgo::Assert(n.v > 0);
    ::std::get<0>(ret) = ::ppgo::tp_int{static_cast<int64_t>(::lom::RandN(static_cast<uint64_t>(n.v)))};
    return nullptr;
}

::ppgo::Exc::Ptr func_fast_rand_n(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int n)
{
    ::ppgo::Assert(n.v > 0);
    static thread_local uint64_t r = static_cast<uint64_t>(::lom::NowUS());
    r = r * 1000003 + 1;
    ::std::get<0>(ret) = ::ppgo::tp_int{static_cast<int64_t>(r % static_cast<uint64_t>(n.v))};
    return nullptr;
}

}

}
