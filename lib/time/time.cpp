#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_time(::std::tuple<::ppgo::tp_float> &ret)
{
    ::std::get<0>(ret) = ::ppgo::util::NowUS() / 1e6;
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
