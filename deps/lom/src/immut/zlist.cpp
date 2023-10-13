#include "../internal.h"

namespace lom
{

namespace immut
{

std::shared_ptr<Str::Buf> ZList::EmptyBuf()
{
    static std::shared_ptr<Str::Buf> empty_buf = std::make_shared<Str::Buf>();
    return empty_buf;
}

GoSlice<StrSlice> ZList::Parse(ssize_t limit) const
{
    auto count = limit <= 0 || limit > StrCount() ? StrCount() : limit;
    GoSlice<StrSlice> r(0, count);

    const char *p = z_->Data();
    auto sz = z_->Len();
    while (count > 0 && sz > 0)
    {
        int64_t n;
        Assert(::lom::var_int::Decode(p, sz, n) && n >= 0 && n <= sz);
        r = r.Append(StrSlice(p, n));
        -- count;
        p += n;
        sz -= n;
    }

    return r;
}

StrSlice ZList::FirstStr() const
{
    const char *p = z_->Data();
    auto sz = z_->Len();
    Assert(sz > 0);

    int64_t n;
    Assert(::lom::var_int::Decode(p, sz, n) && n >= 0 && n <= sz);
    return StrSlice(p, n);
}

std::function<bool (StrSlice &)> ZList::NewForwardIterator() const
{
    return [z = z_, p = static_cast<const char *>(z_->Data()), sz = z_->Len()] (StrSlice &s) mutable -> bool {
        static_cast<void>(z); //holder
        if (sz == 0)
        {
            return false;
        }
        int64_t n;
        Assert(::lom::var_int::Decode(p, sz, n) && n >= 0 && n <= sz);
        s = StrSlice(p, n);
        p += n;
        sz -= n;
        return true;
    };
}

ZList ZList::Append(StrSlice s) const
{
    auto len_enc = ::lom::var_int::Encode(s.Len());

    auto b = std::make_shared<Str::Buf>(0, z_->Len() + len_enc.Len() + s.Len());
    b->Append(z_->Data(), z_->Len());
    b->Append(len_enc);
    b->Append(s);
    Assert(b->Len() == b->Cap());

    return ZList(str_count_ + 1, std::move(b));
}

ZList ZList::Extend(const ZList &zl) const
{
    auto b = std::make_shared<Str::Buf>(0, z_->Len() + zl.z_->Len());
    b->Append(z_->Data(), z_->Len());
    b->Append(zl.z_->Data(), zl.z_->Len());
    Assert(b->Len() == b->Cap());

    return ZList(str_count_ + zl.str_count_, std::move(b));
}

ZList ZList::Extend(const GoSlice<StrSlice> &gs) const
{
    auto count = gs.Len();

    ssize_t total_len = z_->Len();
    auto len_enc_gs = gs.Map<Str>([&total_len] (const StrSlice &s) -> Str {
        auto s_len = s.Len();
        auto len_enc = ::lom::var_int::Encode(s_len);
        total_len += len_enc.Len() + s_len;
        return len_enc;
    });

    auto b = std::make_shared<Str::Buf>(0, total_len);
    b->Append(z_->Data(), z_->Len());
    for (ssize_t i = 0; i < count; ++ i)
    {
        b->Append(len_enc_gs.At(i));
        b->Append(gs.At(i));
    }
    Assert(b->Len() == b->Cap());

    return ZList(str_count_ + count, std::move(b));
}

static ssize_t ZHash(const std::shared_ptr<Str::Buf> &z)
{
    auto p = z->Data();
    auto len = z->Len();
    ::lom::hash::BKDRHash h(len, 25222951875809);
    h.Update(p, len);
    return h.Hash();
}

LOM_ERR ZList::DumpTo(const ::lom::io::BufWriter::Ptr &bw, bool need_flush) const
{
    auto write_int =
        [&bw] (int64_t n) -> LOM_ERR {
            auto enc = ::lom::var_int::Encode(n);
            return bw->WriteAll(enc.Data(), enc.Len());
        }
    ;

    LOM_RET_ON_ERR(write_int(ZHash(z_)));
    LOM_RET_ON_ERR(write_int(z_->Len()));
    LOM_RET_ON_ERR(bw->WriteAll(z_->Data(), z_->Len()));
    if (need_flush)
    {
        LOM_RET_ON_ERR(bw->Flush());
    }
    return nullptr;
}

LOM_ERR ZList::LoadFrom(const ::lom::io::BufReader::Ptr &br, ZList &zl)
{
    int64_t h;
    LOM_RET_ON_ERR(::lom::var_int::LoadFrom(br, h));
    int64_t len;
    LOM_RET_ON_ERR(::lom::var_int::LoadFrom(br, len));
    auto z = std::make_shared<Str::Buf>(len);
    LOM_RET_ON_ERR(br->ReadFull(z->Data(), len));
    if (h != ZHash(z))
    {
        LOM_RET_ERR("hash mismatch");
    }

    ssize_t str_count = 0;
    const char *p = z->Data();
    auto sz = z->Len();
    while (sz > 0)
    {
        int64_t n;
        if (!(::lom::var_int::Decode(p, sz, n) && n >= 0 && n < sz))
        {
            LOM_RET_ERR("invalid zlist data");
        }
        ++ str_count;
        p += n + 1;
        sz -= n + 1;
    }

    zl = ZList(str_count, std::move(z));
    return nullptr;
}

}

}
