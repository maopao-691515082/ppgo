#include "ppgo.h"

#ifdef PPGO_THIS_MOD
#error macro PPGO_THIS_MOD redefined
#endif
#define PPGO_THIS_MOD mod6D07

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_throw(::std::tuple<> &ret, ::ppgo::Any::Ptr a)
{
    return ::ppgo::Exc::New(a);
}

}

}

#undef PPGO_THIS_MOD
