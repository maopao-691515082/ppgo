#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_time_us(::std::tuple<::ppgo::tp_int> &ret)
{
    ::std::get<0>(ret) = ::lom::NowUS();
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
