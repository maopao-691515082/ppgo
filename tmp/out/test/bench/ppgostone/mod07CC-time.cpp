#include "ppgo.h"

#ifdef PPGO_THIS_MOD
#error macro PPGO_THIS_MOD redefined
#endif
#define PPGO_THIS_MOD mod07CC

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

#undef PPGO_THIS_MOD
