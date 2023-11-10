#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_BufReader::method_br_init(::std::tuple<> &ret, ::ppgo::tp_int buf_sz)
{
    ::ppgo::Vec<::ppgo::tp_byte> b;
    nas.br = ::lom::io::BufReader::New(
        [b, r = attr_r] (char *buf, ssize_t sz, ssize_t &rsz) -> LOM_ERR {
            ssize_t begin = buf - reinterpret_cast<char *>(b.GetElemPtr(0));
            ssize_t end = begin + sz;
            ::std::tuple<::ppgo::tp_int> read_ret;
            auto exc = r->method_read(read_ret, ::ppgo::VecView<::ppgo::tp_byte>(b, begin, end));
            if (exc)
            {
                return ::ppgo::Exc::WrapToLomErr(exc);
            }
            rsz = std::get<0>(read_ret);
            return nullptr;
        },
        buf_sz,
        [b] (ssize_t sz) -> char * {
            b.Resize(sz);
            return reinterpret_cast<char *>(b.GetElemPtr(0));
        }
    );

    return nullptr;
}

::ppgo::Exc::Ptr cls_BufReader::method_byte_count_read(::std::tuple<::ppgo::tp_int> &ret)
{
    std::get<0>(ret) = nas.br->ByteCountRead();
    return nullptr;
}

::ppgo::Exc::Ptr cls_BufReader::method_wait(::std::tuple<::ppgo::tp_int> &ret)
{
    ssize_t rsz;
    auto err = nas.br->Wait(rsz);
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    std::get<0>(ret) = rsz;
    return nullptr;
}

::ppgo::Exc::Ptr cls_BufReader::method_br_read(
    ::std::tuple<::ppgo::tp_int> &ret, ::ppgo::VecView<::ppgo::tp_byte> b)
{
    ssize_t rsz;
    auto err = nas.br->Read(reinterpret_cast<char *>(b.GetElemPtr(0)), b.Len(), rsz);
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    std::get<0>(ret) = rsz;
    return nullptr;
}

::ppgo::Exc::Ptr cls_BufReader::method_br_read_until(
    ::std::tuple<::ppgo::tp_int> &ret,
    ::ppgo::tp_byte c, ::ppgo::VecView<::ppgo::tp_byte> b, ::ppgo::tp_bool allow_eof)
{
    ssize_t rsz = -1;
    auto err = nas.br->ReadUntil(static_cast<char>(c), reinterpret_cast<char *>(b.GetElemPtr(0)), b.Len(), rsz);
    if (err)
    {
        if (err == ::lom::io::UnexpectedEOF() && rsz >= 0 && allow_eof)
        {
            std::get<0>(ret) = rsz;
            return nullptr;
        }
        return ::ppgo::Exc::FromLomErr(err);
    }
    std::get<0>(ret) = rsz;
    return nullptr;
}

::ppgo::Exc::Ptr cls_BufReader::method_br_read_full(
    ::std::tuple<::ppgo::tp_int> &ret, ::ppgo::VecView<::ppgo::tp_byte> b, ::ppgo::tp_bool allow_eof)
{
    ssize_t rsz = -1;
    auto err = nas.br->ReadFull(reinterpret_cast<char *>(b.GetElemPtr(0)), b.Len(), &rsz);
    if (err)
    {
        if (err == ::lom::io::UnexpectedEOF() && rsz >= 0 && allow_eof)
        {
            std::get<0>(ret) = rsz;
            return nullptr;
        }
        return ::ppgo::Exc::FromLomErr(err);
    }
    std::get<0>(ret) = rsz;
    return nullptr;
}

::ppgo::Exc::Ptr cls_BufWriter::method_bw_init(::std::tuple<> &ret, ::ppgo::tp_int buf_sz)
{
    ::ppgo::Vec<::ppgo::tp_byte> b;
    nas.bw = ::lom::io::BufWriter::New(
        [b, w = attr_w] (const char *buf, ssize_t sz, ssize_t &wsz) -> LOM_ERR {
            ssize_t begin = buf - reinterpret_cast<const char *>(b.GetElemPtr(0));
            ssize_t end = begin + sz;
            ::std::tuple<> write_ret;
            auto exc = w->method_write(write_ret, ::ppgo::VecView<::ppgo::tp_byte>(b, begin, end));
            if (exc)
            {
                return ::ppgo::Exc::WrapToLomErr(exc);
            }
            wsz = sz;
            return nullptr;
        },
        buf_sz,
        [b] (ssize_t sz) -> char * {
            b.Resize(sz);
            return reinterpret_cast<char *>(b.GetElemPtr(0));
        }
    );

    return nullptr;
}

::ppgo::Exc::Ptr cls_BufWriter::method_byte_count_written(::std::tuple<::ppgo::tp_int> &ret)
{
    std::get<0>(ret) = nas.bw->ByteCountWritten();
    return nullptr;
}

::ppgo::Exc::Ptr cls_BufWriter::method_write(::std::tuple<> &ret, ::ppgo::VecView<::ppgo::tp_byte> b)
{
    auto err = nas.bw->WriteAll(reinterpret_cast<const char *>(b.GetElemPtr(0)), b.Len());
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    return nullptr;
}

::ppgo::Exc::Ptr cls_BufWriter::method_write_str(::std::tuple<> &ret, ::ppgo::tp_string s)
{
    auto err = nas.bw->WriteAll(s.Data(), s.Len());
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    return nullptr;
}

::ppgo::Exc::Ptr cls_BufWriter::method_flush(::std::tuple<> &ret)
{
    auto err = nas.bw->Flush();
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
