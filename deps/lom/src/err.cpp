#include "internal.h"

namespace lom
{

Str Err::FmtWithTB() const
{
    Str::Buf b;
    b.Append("Error: ");
    b.Append(Msg());
    b.Append("\n");
    for (auto const &s : tb_)
    {
        b.Append("  from ");
        b.Append(s);
        b.Append("\n");
    }
    return Str(std::move(b));
}

bool Err::IsSysCallErr(::std::optional<int> code, ::std::optional<int> eno)
{
    auto sys_call_err = dynamic_cast<SysCallErr *>(this);
    if (!sys_call_err)
    {
        return false;
    }
    if (code && sys_call_err->Code() != *code)
    {
        return false;
    }
    if (eno && sys_call_err->Errno() != *eno)
    {
        return false;
    }

    return true;
}

class SimpleErr : public Err
{
    Str msg_;

public:

    SimpleErr(const Str &msg) : msg_(msg)
    {
    }

    virtual Str Msg() const override
    {
        return msg_;
    }
};

LOM_ERR Err::FromStr(const Str &msg, CodePos _cp)
{
    auto err = LOM_ERR(new SimpleErr(msg));
    if (_cp.Valid())
    {
        err->PushTB(_cp);
    }
    return err;
}

}
