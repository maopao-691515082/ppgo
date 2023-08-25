#include "../internal.h"

namespace lom
{

namespace thread
{

::lom::Err::Ptr SetThreadName(const char *name)
{
    return
        prctl(PR_SET_NAME, name) == -1 ?
            ::lom::Err::Sprintf("set thread name failed, <errno=%d>", errno) :
            nullptr;
}

}

}
