#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_TCPListener::method_init_impl(::std::tuple<> &ret, ::ppgo::tp_u16 port)
{
    auto listener = new ::lom::fiber::Listener;
    attr_l = reinterpret_cast<::ppgo::tp_uptr>(listener);

    *listener = ::lom::fiber::ListenTCP(port);
    if (!listener->Valid())
    {
        return ::ppgo::Exc::Sprintf(
            "failed to listen on port [%lld]: %s", static_cast<long long>(port), ::lom::Err().CStr());
    }
    return nullptr;
}

::ppgo::Exc::Ptr cls_TCPListener::method_deinit(::std::tuple<> &ret)
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

::ppgo::Exc::Ptr cls_TCPListener::method_accept_impl(
    ::std::tuple<std::shared_ptr<cls_TCPConn>> &ret, ::ppgo::tp_int timeout)
{
    auto listener = reinterpret_cast<::lom::fiber::Listener *>(attr_l);

    auto conn = listener->Accept(timeout);
    if (!conn.Valid())
    {
        return ::ppgo::Exc::Sprintf("failed to accept: %s", ::lom::Err().CStr());
    }

    auto tcp_conn = std::make_shared<cls_TCPConn>();
    auto c = new ::lom::fiber::Conn;
    *c = conn;
    tcp_conn->attr_c = reinterpret_cast<void *>(c);
    std::get<0>(ret) = tcp_conn;
    return nullptr;
}

}

}
