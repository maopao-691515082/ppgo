#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_Listener::method_deinit(::std::tuple<> &ret)
{
    if (attr_l)
    {
        auto listener = reinterpret_cast<::lom::fiber::Listener *>(attr_l);
        listener->Close();
        delete listener;
        attr_l = nullptr;
    }
    return nullptr;
}

::ppgo::Exc::Ptr cls_Listener::method_accept(::std::tuple<std::shared_ptr<cls_Conn>> &ret)
{
    auto listener = reinterpret_cast<::lom::fiber::Listener *>(attr_l);

    ::lom::fiber::Conn lom_conn;
    auto err = listener->Accept(lom_conn);
    if (err)
    {
        auto exc = ::ppgo::PPGO_THIS_MOD::ExcFromLomErr(err);
        if (exc)
        {
            return exc;
        }
        return ::ppgo::Exc::Sprintf("failed to accept: %s", err->Msg().CStr());
    }

    auto conn = std::make_shared<cls_Conn>();
    auto c = new ::lom::fiber::Conn;
    conn->attr_c = reinterpret_cast<::ppgo::tp_uptr>(c);
    *c = lom_conn;

    std::get<0>(ret) = conn;
    return nullptr;
}

::ppgo::Exc::Ptr cls_Listener::method_serve_impl(
    ::std::tuple<> &ret, std::shared_ptr<intf_ClientWorker> cw, ::ppgo::tp_uint worker_count)
{
    auto listener = reinterpret_cast<::lom::fiber::Listener *>(attr_l);

    auto err = listener->Serve(worker_count, [cw] (::lom::fiber::Conn lom_conn) {
        auto conn = std::make_shared<cls_Conn>();
        auto c = new ::lom::fiber::Conn;
        conn->attr_c = reinterpret_cast<::ppgo::tp_uptr>(c);
        *c = lom_conn;

        std::tuple<> r;
        auto exc = cw->method_run(r, conn);
        if (exc)
        {
            auto ftb = exc->FormatWithTB();
            ::ppgo::util::OutputUnexpectedErrMsg(::lom::Sprintf(
                "Uncached Exc in fiber.Listener.ClientWorker.run: %s\n", ftb.Data()));
        }
    });
    if (err)
    {
        return ::ppgo::Exc::Sprintf("fiber.Listener.serve exit: %s", err->Msg().CStr());
    }

    return nullptr;
}

::ppgo::Exc::Ptr func_listen_tcp_impl(
    ::std::tuple<std::shared_ptr<cls_Listener>> &ret, ::ppgo::tp_u16 port)
{
    auto listener = std::make_shared<cls_Listener>();
    auto l = new ::lom::fiber::Listener;
    listener->attr_l = reinterpret_cast<::ppgo::tp_uptr>(l);

    auto err = ::lom::fiber::ListenTCP(port, *l);
    if (err)
    {
        return ::ppgo::Exc::Sprintf(
            "failed to listen tcp on port [%lld]: %s", static_cast<long long>(port), err->Msg().CStr());
    }

    std::get<0>(ret) = listener;
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
