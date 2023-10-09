#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_rand(::std::tuple<::ppgo::tp_float> &ret)
{
    ::std::get<0>(ret) = ::lom::TLSRandGenerator()->Rand01<::ppgo::tp_float>();
    return nullptr;
}

::ppgo::Exc::Ptr func_rand_int(::std::tuple<::ppgo::tp_int> &ret)
{
    ::std::get<0>(ret) = static_cast<::ppgo::tp_int>(
        ::lom::RandU64() & static_cast<uint64_t>(::lom::kInt64Max));
    return nullptr;
}

::ppgo::Exc::Ptr func_rand_int_fast(::std::tuple<::ppgo::tp_int> &ret)
{
    static thread_local uint64_t r = static_cast<uint64_t>(::lom::NowUS());
    r = r * 1000003 + 1;
    ::std::get<0>(ret) = static_cast<::ppgo::tp_int>(
        r & static_cast<uint64_t>(::lom::kInt64Max));
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
