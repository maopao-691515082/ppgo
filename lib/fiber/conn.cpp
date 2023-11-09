#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_Conn::method_read_impl(
    ::std::tuple<::ppgo::tp_int> &ret, ::ppgo::VecView<::ppgo::tp_byte> b)
{
    ssize_t n;
    auto err = nas.conn.Read(reinterpret_cast<char *>(b.GetElemPtr(0)), b.Len(), n);
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
    ::std::tuple<> &ret, ::ppgo::VecView<::ppgo::tp_byte> b)
{
    auto err = nas.conn.WriteAll(reinterpret_cast<const char *>(b.GetElemPtr(0)), b.Len());
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

::ppgo::Exc::Ptr cls_Conn::method_write_str_impl(::std::tuple<> &ret, ::ppgo::tp_string s)
{
    auto err = nas.conn.WriteAll(s.Data(), s.Len());
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
    ::std::tuple<std::shared_ptr<cls_Conn>> &ret, ::ppgo::tp_string ipv4, ::ppgo::tp_u16 port)
{
    auto conn = std::make_shared<cls_Conn>();
    auto err = ::lom::fiber::ConnectTCP(ipv4.Data(), port, conn->nas.conn);
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
