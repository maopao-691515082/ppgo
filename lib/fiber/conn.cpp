#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_TCPConn::method_init(
    ::std::tuple<> &ret, ::ppgo::tp_string ipv4, ::ppgo::tp_int port, ::ppgo::tp_int timeout_ms)
{
    if (ipv4.RawStr().ContainsChar('\0'))
    {
        return ::ppgo::Exc::Sprintf("invalid ipv4 address: contains NUL char");
    }

    if (port < 0 || port > 65535)
    {
        return ::ppgo::Exc::Sprintf("invalid port number [%lld]", static_cast<long long>(port));
    }

    auto conn = new ::lom::fiber::Conn;
    attr_c = reinterpret_cast<::ppgo::tp_uptr>(conn);

    *conn = ::lom::fiber::ConnectTCP(ipv4.Data(), static_cast<uint16_t>(port), timeout_ms);
    if (!conn->Valid())
    {
        return ::ppgo::Exc::Sprintf(
            "connect to [%s:%lld] failed: %s",
            ipv4.Data(), static_cast<long long>(port), ::lom::Err().CStr());
    }
    return nullptr;
}

::ppgo::Exc::Ptr cls_TCPConn::method_deinit(::std::tuple<> &ret)
{
    if (attr_c)
    {
        auto conn = reinterpret_cast<::lom::fiber::Conn *>(attr_c);
        conn->Close();
        delete conn;
        attr_c = nullptr;
    }
    return nullptr;
}

::ppgo::Exc::Ptr cls_TCPConn::method_read_timeout(
    ::std::tuple<::ppgo::tp_int> &ret,
    ::ppgo::Vec<::ppgo::tp_byte> b, ::ppgo::tp_int start, ::ppgo::tp_int timeout_ms)
{
    auto b_len = b.Len();
    if (start < 0 || start >= b_len)
    {
        return ::ppgo::Exc::Sprintf("start out of range");
    }

    auto conn = reinterpret_cast<::lom::fiber::Conn *>(attr_c);
    auto n = conn->Read(reinterpret_cast<char *>(&b.GetRef(start)), b_len - start, timeout_ms);
    if (n < 0)
    {
        return ::ppgo::Exc::Sprintf("read failed: %s", ::lom::Err().CStr());
    }

    std::get<0>(ret) = n;
    return nullptr;
}

::ppgo::Exc::Ptr cls_TCPConn::method_write_timeout(
    ::std::tuple<> &ret,
    ::ppgo::Vec<::ppgo::tp_byte > b, ::ppgo::tp_int start, ::ppgo::tp_int len, ::ppgo::tp_int timeout_ms)
{
    auto b_len = b.Len();
    if (start < 0 || start >= b_len || len < 0 || len > b_len - start)
    {
        return ::ppgo::Exc::Sprintf("start or len out of range");
    }

    auto conn = reinterpret_cast<::lom::fiber::Conn *>(attr_c);
    if (conn->WriteAll(reinterpret_cast<const char *>(&b.GetRef(start)), len, timeout_ms) < 0)
    {
        return ::ppgo::Exc::Sprintf("write failed: %s", ::lom::Err().CStr());
    }

    return nullptr;
}

}

}
