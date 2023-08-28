#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_float_min(::std::tuple<::ppgo::tp_float> &ret)
{
    std::get<0>(ret) = std::numeric_limits<::ppgo::tp_float>::min();
    return nullptr;
}

::ppgo::Exc::Ptr func_float_max(::std::tuple<::ppgo::tp_float> &ret)
{
    std::get<0>(ret) = std::numeric_limits<::ppgo::tp_float>::max();
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
