#include "internal.h"

namespace lom
{

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

bool IsSysCallErr(LOM_ERR err, ::std::optional<int> code, ::std::optional<int> eno)
{
    auto sys_call_err = dynamic_cast<SysCallErr *>(err.get());
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

}
