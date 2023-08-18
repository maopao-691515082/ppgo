#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

static void new_fiber_impl(const std::shared_ptr<intf_Fiber> &f)
{
    ::lom::fiber::Create([f] () {
        ::std::tuple<> ret;
        auto exc = f->method_run(ret);
        if (exc)
        {
            auto ftb = exc->FormatWithTB();
            fprintf(stderr, "<Fiber> Uncached Exc: %s\n", ftb.Data());
            _exit(2);
        }
    });
}

::ppgo::Exc::Ptr func_run(::std::tuple<> &ret, std::shared_ptr<intf_Fiber> f)
{
    ::lom::fiber::MustInit();
    new_fiber_impl(f);
    ::lom::fiber::Run();
    return nullptr;
}

::ppgo::Exc::Ptr func_new(::std::tuple<> &ret, std::shared_ptr<intf_Fiber> f)
{
    new_fiber_impl(f);
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
