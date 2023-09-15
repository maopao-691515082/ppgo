#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_Mutex::method_lock(::std::tuple<> &ret)
{
    na_l.lock();
    return nullptr;
}

::ppgo::Exc::Ptr cls_Mutex::method_unlock(::std::tuple<> &ret)
{
    na_l.unlock();
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
