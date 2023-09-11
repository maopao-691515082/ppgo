#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_throw(::std::tuple<> &ret, ::ppgo::Any::Ptr a)
{
    return ::ppgo::Exc::New(a);
}

::ppgo::Exc::Ptr func_rethrow(::std::tuple<> &ret, ::ppgo::Any::Ptr a, ::ppgo::tp_string tb)
{
    return ::ppgo::Exc::NewRethrow(a, tb);
}

}

std::shared_ptr<Exc> NewTypeAssertionException()
{
    return Exc::New(::ppgo::Any::Ptr(new ::ppgo::PPGO_THIS_MOD::cls_TypeAssertionException));
}

}

#pragma ppgo undef-THIS_MOD
