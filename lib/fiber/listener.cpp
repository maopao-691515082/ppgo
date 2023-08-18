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

::ppgo::Exc::Ptr cls_Listener::method_accept_impl(
    ::std::tuple<std::shared_ptr<cls_Conn>> &ret, ::ppgo::tp_int timeout)
{
    auto listener = reinterpret_cast<::lom::fiber::Listener *>(attr_l);

    auto lom_conn = listener->Accept(timeout);
    if (!lom_conn.Valid())
    {
        return ::ppgo::Exc::Sprintf("failed to accept: %s", ::lom::Err().CStr());
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

    listener->Serve(worker_count, [cw] (::lom::fiber::Conn lom_conn) {
        auto conn = std::make_shared<cls_Conn>();
        auto c = new ::lom::fiber::Conn;
        conn->attr_c = reinterpret_cast<::ppgo::tp_uptr>(c);
        *c = lom_conn;

        std::tuple<> r;
        auto exc = cw->method_run(r, conn);
        if (exc)
        {
            auto ftb = exc->FormatWithTB();
            fprintf(stderr, "<Fiber> Uncached Exc: %s\n", ftb.Data());
            _exit(2);
        }
    });

    return nullptr;
}

::ppgo::Exc::Ptr func_listen_tcp_impl(
    ::std::tuple<std::shared_ptr<cls_Listener>> &ret, ::ppgo::tp_u16 port)
{
    auto listener = std::make_shared<cls_Listener>();
    auto l = new ::lom::fiber::Listener;
    listener->attr_l = reinterpret_cast<::ppgo::tp_uptr>(l);

    *l = ::lom::fiber::ListenTCP(port);
    if (!l->Valid())
    {
        return ::ppgo::Exc::Sprintf(
            "failed to listen tcp on port [%lld]: %s", static_cast<long long>(port), ::lom::Err().CStr());
    }

    std::get<0>(ret) = listener;
    return nullptr;
}

}

}
