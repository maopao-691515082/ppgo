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
            ::ppgo::util::OutputUnexpectedErrMsg(::lom::Sprintf(
                "Uncached Exc in fiber main: %s\n", ftb.Data()));
            _exit(2);
        }
    });
}

::ppgo::Exc::Ptr func_run(::std::tuple<> &ret, std::shared_ptr<intf_Fiber> f)
{
    ::lom::fiber::MustInit();
    new_fiber_impl(f);
    auto err = ::lom::fiber::Run();
    if (err)
    {
        return ::ppgo::Exc::Sprintf("fiber.run exit: %s", err->Msg().CStr());
    }
    return nullptr;
}

::ppgo::Exc::Ptr func_new(::std::tuple<> &ret, std::shared_ptr<intf_Fiber> f)
{
    new_fiber_impl(f);
    return nullptr;
}

::ppgo::Exc::Ptr cls_Ctx::method_call(
    ::std::tuple<> &, std::function<::ppgo::Exc::Ptr (::std::tuple<> &)> f)
{
    auto err = ::lom::fiber::Ctx(attr_timeout_ms).Call(
        [&] () -> ::lom::Err::Ptr {
            ::std::tuple<> ret;
            auto exc = f(ret);
            if (exc)
            {
                return ::ppgo::Exc::WrapToLomErr(exc);
            }
            return nullptr;
        }
    );
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    return nullptr;
}

::ppgo::Exc::Ptr func_check_ctx(::std::tuple<> &ret)
{
    auto err = ::lom::fiber::Ctx::Check();
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
