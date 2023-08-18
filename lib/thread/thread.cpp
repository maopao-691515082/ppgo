#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_new(::std::tuple<> &ret, std::shared_ptr<intf_Thread> t)
{
    std::thread([t] () {
        ::std::tuple<> r;
        auto exc = t->method_run(r);
        if (exc)
        {
            auto ftb = exc->FormatWithTB();
            ::ppgo::util::OutputUnexpectedErrMsg(::lom::Sprintf(
                "Uncached Exc in thread main: %s\n", ftb.Data()));
            _exit(2);
        }
    }).detach();
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
