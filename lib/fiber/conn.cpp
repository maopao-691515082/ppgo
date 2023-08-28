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
    ::std::tuple<::ppgo::tp_int> &ret, ::ppgo::VecView<::ppgo::tp_byte> b, ::ppgo::tp_int timeout_ms)
{
    auto conn = reinterpret_cast<::lom::fiber::Conn *>(attr_c);
    ssize_t n;
    auto err = conn->Read(reinterpret_cast<char *>(&b.GetRef(0)), b.Len(), n, timeout_ms);
    if (err)
    {
        auto exc = ::ppgo::PPGO_THIS_MOD::ExcFromLomErr(err);
        if (exc)
        {
            return exc;
        }
        return ::ppgo::Exc::Sprintf("read failed: %s", err->Msg().CStr());
    }

    std::get<0>(ret) = n;
    return nullptr;
}

::ppgo::Exc::Ptr cls_Conn::method_write_impl(
    ::std::tuple<> &ret, ::ppgo::VecView<::ppgo::tp_byte> b, ::ppgo::tp_int timeout_ms)
{
    auto conn = reinterpret_cast<::lom::fiber::Conn *>(attr_c);
    auto err = conn->WriteAll(reinterpret_cast<const char *>(&b.GetRef(0)), b.Len(), timeout_ms);
    if (err)
    {
        auto exc = ::ppgo::PPGO_THIS_MOD::ExcFromLomErr(err);
        if (exc)
        {
            return exc;
        }
        return ::ppgo::Exc::Sprintf("write failed: %s", err->Msg().CStr());
    }
    return nullptr;
}

::ppgo::Exc::Ptr cls_Conn::method_write_str_impl(
    ::std::tuple<> &ret, ::ppgo::tp_string s, ::ppgo::tp_int timeout_ms)
{
    auto conn = reinterpret_cast<::lom::fiber::Conn *>(attr_c);
    auto err = conn->WriteAll(s.Data(), s.Len(), timeout_ms);
    if (err)
    {
        auto exc = ::ppgo::PPGO_THIS_MOD::ExcFromLomErr(err);
        if (exc)
        {
            return exc;
        }
        return ::ppgo::Exc::Sprintf("write failed: %s", err->Msg().CStr());
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

    auto err = ::lom::fiber::ConnectTCP(ipv4.Data(), port, *c, timeout_ms);
    if (err)
    {
        auto exc = ::ppgo::PPGO_THIS_MOD::ExcFromLomErr(err);
        if (exc)
        {
            return exc;
        }
        return ::ppgo::Exc::Sprintf(
            "connect tcp to [%s:%lld] failed: %s",
            ipv4.Data(), static_cast<long long>(port), err->Msg().CStr());
    }

    std::get<0>(ret) = conn;
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
