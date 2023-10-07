#include "../internal.h"

namespace lom
{

namespace io
{

LOM_ERR UnexpectedEOF()
{
    static LOM_ERR err = ::lom::Err::FromStr("Unexpected EOF");
    return err;
}

}

}
