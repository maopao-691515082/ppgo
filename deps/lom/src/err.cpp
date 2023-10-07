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

}
