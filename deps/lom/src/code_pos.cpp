#include "internal.h"

namespace lom
{

::lom::Str CodePos::Str() const
{
    return
        Valid() ?
            Sprintf(
                "File [%s] Line [%d] Func [%s]",
                ::lom::os::NormPath(file_name_).CStr(), line_num_, func_name_) :
            "INVALID CODE-POS";
}

}
