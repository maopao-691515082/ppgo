#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_TCPListener::method_init(::std::tuple<> &ret, ::ppgo::tp_int port)
{
    if (port < 0 || port > 65535)
    {
        return ::ppgo::Exc::Sprintf("invalid port number [%lld]", static_cast<long long>(port));
    }

    auto listener = new ::lom::fiber::Listener;
    attr_l = reinterpret_cast<::ppgo::tp_uptr>(listener);

    *listener = ::lom::fiber::ListenTCP(static_cast<uint16_t>(port));
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

::ppgo::Exc::Ptr cls_TCPListener::method_accept(::std::tuple<std::shared_ptr<cls_TCPConn>> &ret)
{
    auto listener = reinterpret_cast<::lom::fiber::Listener *>(attr_l);

    auto conn = listener->Accept();
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
