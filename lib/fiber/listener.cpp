#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_Listener::method_accept(::std::tuple<std::shared_ptr<cls_Conn>> &ret)
{
    ::lom::fiber::Conn lom_conn;
    auto err = nas.listener.Accept(lom_conn);
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
    conn->nas.conn = lom_conn;

    std::get<0>(ret) = conn;
    return nullptr;
}

::ppgo::Exc::Ptr cls_Listener::method_serve_impl(
    ::std::tuple<> &ret, std::shared_ptr<intf_ClientWorker> cw, ::ppgo::tp_uint worker_count)
{
    auto err = nas.listener.Serve(worker_count, [cw] (::lom::fiber::Conn lom_conn) {
        auto conn = std::make_shared<cls_Conn>();
        conn->nas.conn = lom_conn;

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

    auto err = ::lom::fiber::ListenTCP(port, listener->nas.listener);
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
