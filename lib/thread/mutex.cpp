#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_Mutex::method_init(::std::tuple<> &ret)
{
    attr_l = reinterpret_cast<::ppgo::tp_uptr>(new std::mutex);
    return nullptr;
}

::ppgo::Exc::Ptr cls_Mutex::method_deinit(::std::tuple<> &ret)
{
    delete reinterpret_cast<std::mutex *>(attr_l);
    attr_l = nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr cls_Mutex::method_lock(::std::tuple<> &ret)
{
    auto l = reinterpret_cast<std::mutex *>(attr_l);
    l->lock();
    return nullptr;
}

::ppgo::Exc::Ptr cls_Mutex::method_unlock(::std::tuple<> &ret)
{
    auto l = reinterpret_cast<std::mutex *>(attr_l);
    l->unlock();
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
