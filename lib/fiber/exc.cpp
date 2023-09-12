#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr ExcFromLomErr(::lom::Err::Ptr err)
{
    auto sys_call_err = dynamic_cast<::lom::SysCallErr *>(err.RawPtr());
    if (sys_call_err)
    {
        switch (sys_call_err->Code())
        {
            case ::lom::fiber::err_code::kTimeout:
            {
                return ::ppgo::Exc::New(std::static_pointer_cast<::ppgo::Any>(gv_kExcTimeout));
            }
            case ::lom::fiber::err_code::kClosed:
            {
                return ::ppgo::Exc::New(std::static_pointer_cast<::ppgo::Any>(gv_kExcClosed));
            }
            case ::lom::fiber::err_code::kCanceled:
            {
                return ::ppgo::Exc::New(std::static_pointer_cast<::ppgo::Any>(gv_kExcCanceled));
            }
            case ::lom::fiber::err_code::kConnReset:
            {
                return ::ppgo::Exc::New(std::static_pointer_cast<::ppgo::Any>(gv_kExcConnReset));
            }
        }
    }
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
