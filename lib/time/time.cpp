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

::ppgo::Exc::Ptr func_sleep(::std::tuple<> &ret, ::ppgo::tp_float sec)
{
    if (sec > 0)
    {
        if (sec > 1e12)
        {
            sec = 1e12;
        }
        std::chrono::microseconds us{static_cast<uint64_t>(sec * 1e6)};
        std::this_thread::sleep_for(us);
    }
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
