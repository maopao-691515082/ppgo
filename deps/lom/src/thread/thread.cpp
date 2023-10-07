#include "../internal.h"

namespace lom
{

namespace thread
{

LOM_ERR SetThreadName(const char *name)
{
    if (::prctl(PR_SET_NAME, name) == -1)
    {
        LOM_RET_SYS_CALL_ERR("set thread name `%s` failed", name);
    }
    return nullptr;
}

}

}
