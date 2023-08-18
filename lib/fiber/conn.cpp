#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_Conn::method_deinit(::std::tuple<> &ret)
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

::ppgo::Exc::Ptr cls_Conn::method_read_impl(
    ::std::tuple<::ppgo::tp_int> &ret,
    ::ppgo::Vec<::ppgo::tp_byte> b, ::ppgo::tp_int start, ::ppgo::tp_int len, ::ppgo::tp_int timeout_ms)
{
    auto conn = reinterpret_cast<::lom::fiber::Conn *>(attr_c);
    auto n = conn->Read(reinterpret_cast<char *>(&b.GetRef(start)), len, timeout_ms);
    if (n < 0)
    {
        return ::ppgo::Exc::Sprintf("read failed: %s", ::lom::Err().CStr());
    }

    std::get<0>(ret) = n;
    return nullptr;
}

::ppgo::Exc::Ptr cls_Conn::method_write_impl(
    ::std::tuple<> &ret,
    ::ppgo::Vec<::ppgo::tp_byte> b, ::ppgo::tp_int start, ::ppgo::tp_int len, ::ppgo::tp_int timeout_ms)
{
    auto conn = reinterpret_cast<::lom::fiber::Conn *>(attr_c);
    if (conn->WriteAll(reinterpret_cast<const char *>(&b.GetRef(start)), len, timeout_ms) < 0)
    {
        return ::ppgo::Exc::Sprintf("write failed: %s", ::lom::Err().CStr());
    }
    return nullptr;
}

::ppgo::Exc::Ptr func_connect_tcp_impl(
    ::std::tuple<std::shared_ptr<cls_Conn>> &ret,
    ::ppgo::tp_string ipv4, ::ppgo::tp_u16 port, ::ppgo::tp_int timeout_ms)
{
    auto conn = std::make_shared<cls_Conn>();
    auto c = new ::lom::fiber::Conn;
    conn->attr_c = reinterpret_cast<::ppgo::tp_uptr>(c);

    *c = ::lom::fiber::ConnectTCP(ipv4.Data(), port, timeout_ms);
    if (!c->Valid())
    {
        return ::ppgo::Exc::Sprintf(
            "connect tcp to [%s:%lld] failed: %s",
            ipv4.Data(), static_cast<long long>(port), ::lom::Err().CStr());
    }

    std::get<0>(ret) = conn;
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
