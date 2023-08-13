#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_init_env(::std::tuple<> &ret)
{
    if (!::lom::fiber::Init())
    {
        return ::ppgo::Exc::Sprintf("init fiber env failed: %s", ::lom::Err().CStr());
    }
    return nullptr;
}

::ppgo::Exc::Ptr func_run(::std::tuple<> &ret)
{
    ::lom::fiber::Run();
    return nullptr;
}

::ppgo::Exc::Ptr func_new(::std::tuple<> &ret, std::shared_ptr<intf_Fiber> f)
{
    ::lom::fiber::Create([f] () {
        ::std::tuple<> r;
        auto exc = f->method_run(r);
        if (exc)
        {
            auto ftb = exc->FormatWithTB();
            fprintf(stderr, "<Fiber> Uncached Exc: %s\n", ftb.Data());
            _exit(2);
        }
    });
    return nullptr;
}

}

}
