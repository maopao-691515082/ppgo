#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_BufReader::method_br_init(::std::tuple<> &ret, ::ppgo::tp_int buf_sz)
{
    auto br = new ::lom::io::BufReader::Ptr;
    attr_br = reinterpret_cast<::ppgo::tp_uptr>(br);

    ::ppgo::Vec<::ppgo::tp_byte> b;
    *br = ::lom::io::BufReader::New(
        [b, r = attr_r] (char *buf, ssize_t sz, ssize_t &rsz) -> ::lom::Err::Ptr {
            ssize_t begin = buf - reinterpret_cast<char *>(&b.GetRef(0));
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
            return reinterpret_cast<char *>(&b.GetRef(0));
        }
    );

    return nullptr;
}

::ppgo::Exc::Ptr cls_BufReader::method_deinit(::std::tuple<> &ret)
{
    if (attr_br)
    {
        auto br = reinterpret_cast<::lom::io::BufReader::Ptr *>(attr_br);
        delete br;
        attr_br = nullptr;
    }
    return nullptr;
}

::ppgo::Exc::Ptr cls_BufReader::method_wait(::std::tuple<::ppgo::tp_int> &ret)
{
    auto br = reinterpret_cast<::lom::io::BufReader::Ptr *>(attr_br);
    ssize_t rsz;
    auto err = (*br)->Wait(rsz);
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
    auto br = reinterpret_cast<::lom::io::BufReader::Ptr *>(attr_br);
    ssize_t rsz;
    auto err = (*br)->Read(reinterpret_cast<char *>(&b.GetRef(0)), b.Len(), rsz);
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
    auto br = reinterpret_cast<::lom::io::BufReader::Ptr *>(attr_br);
    ssize_t rsz = -1;
    auto err = (*br)->ReadUntil(static_cast<char>(c), reinterpret_cast<char *>(&b.GetRef(0)), b.Len(), rsz);
    if (err)
    {
        if (err == ::lom::io::BufReader::UnexpectedEOF() && rsz >= 0 && allow_eof)
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
    auto br = reinterpret_cast<::lom::io::BufReader::Ptr *>(attr_br);
    ssize_t rsz = -1;
    auto err = (*br)->ReadFull(reinterpret_cast<char *>(&b.GetRef(0)), b.Len(), &rsz);
    if (err)
    {
        if (err == ::lom::io::BufReader::UnexpectedEOF() && rsz >= 0 && allow_eof)
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
    auto bw = new ::lom::io::BufWriter::Ptr;
    attr_bw = reinterpret_cast<::ppgo::tp_uptr>(bw);

    ::ppgo::Vec<::ppgo::tp_byte> b;
    *bw = ::lom::io::BufWriter::New(
        [b, w = attr_w] (const char *buf, ssize_t sz, ssize_t &wsz) -> ::lom::Err::Ptr {
            ssize_t begin = buf - reinterpret_cast<const char *>(&b.GetRef(0));
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
            return reinterpret_cast<char *>(&b.GetRef(0));
        }
    );

    return nullptr;
}

::ppgo::Exc::Ptr cls_BufWriter::method_deinit(::std::tuple<> &ret)
{
    if (attr_bw)
    {
        auto bw = reinterpret_cast<::lom::io::BufWriter::Ptr *>(attr_bw);
        delete bw;
        attr_bw = nullptr;
    }
    return nullptr;
}

::ppgo::Exc::Ptr cls_BufWriter::method_write(::std::tuple<> &ret, ::ppgo::VecView<::ppgo::tp_byte> b)
{
    auto bw = reinterpret_cast<::lom::io::BufWriter::Ptr *>(attr_bw);
    auto err = (*bw)->WriteAll(reinterpret_cast<const char *>(&b.GetRef(0)), b.Len());
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    return nullptr;
}

::ppgo::Exc::Ptr cls_BufWriter::method_write_str(::std::tuple<> &ret, ::ppgo::tp_string s)
{
    auto bw = reinterpret_cast<::lom::io::BufWriter::Ptr *>(attr_bw);
    auto err = (*bw)->WriteAll(s.Data(), s.Len());
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    return nullptr;
}

::ppgo::Exc::Ptr cls_BufWriter::method_flush(::std::tuple<> &ret)
{
    auto bw = reinterpret_cast<::lom::io::BufWriter::Ptr *>(attr_bw);
    auto err = (*bw)->Flush();
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
