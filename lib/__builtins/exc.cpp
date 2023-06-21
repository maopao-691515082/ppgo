#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_throw(::std::tuple<> &ret, const ::ppgo::Any::Ptr &a)
{
    return ::ppgo::Exc::New(a);
}

}

std::shared_ptr<Exc> NewTypeAssertionException()
{
    return Exc::New(::ppgo::Any::Ptr(new ::ppgo::PPGO_THIS_MOD::cls_TypeAssertionException));
}

}

#pragma ppgo undef-THIS_MOD
