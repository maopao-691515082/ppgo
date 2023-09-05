#include "../internal.h"

namespace lom
{

namespace immut
{

void ZList::Iterator::ParseV()
{
    auto p = zs_.Data();
    auto sz = zs_.Len();
    if (sz == 0)
    {
        return;
    }

    int64_t n;
    Assert(::lom::var_int::Decode(p, sz, n) && n >= 0 && n < sz);
    v_ = StrSlice(p, n, true);
}

ZList ZList::Append(StrSlice s) const
{
    auto len_enc = ::lom::var_int::Encode(s.Len());

    Str::Buf b(0, z_.Len() + len_enc.Len() + s.Len() + 1);
    b.Append(z_);
    b.Append(len_enc);
    b.Append(s);
    b.Append(StrSlice("\0", 1));
    Assert(b.Len() == b.Cap());

    ZList new_zl;
    new_zl.str_count_ = str_count_ + 1;
    new_zl.z_ = std::move(b);
    return new_zl;
}

ZList ZList::Extend(const ZList &zl) const
{
    ZList new_zl;
    new_zl.str_count_ = str_count_ + zl.str_count_;
    new_zl.z_ = z_.Concat(zl.z_);
    return new_zl;
}

ZList ZList::Extend(const GoSlice<StrSlice> &gs) const
{
    auto count = gs.Len();

    ssize_t total_len = z_.Len();
    auto len_enc_gs = gs.Map<Str>([&total_len] (const StrSlice &s) -> Str {
        auto s_len = s.Len();
        auto len_enc = ::lom::var_int::Encode(s_len);
        total_len += len_enc.Len() + s_len + 1;
        return len_enc;
    });

    Str::Buf b(0, total_len);
    b.Append(z_);
    for (ssize_t i = 0; i < count; ++ i)
    {
        b.Append(len_enc_gs.At(i));
        b.Append(gs.At(i));
        b.Append(StrSlice("\0", 1));
    }
    Assert(b.Len() == b.Cap());

    ZList new_zl;
    new_zl.str_count_ = str_count_ + count;
    new_zl.z_ = std::move(b);
    return new_zl;
}

}

}
