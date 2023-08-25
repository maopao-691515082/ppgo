#include "internal.h"

namespace lom
{

namespace fiber
{

static thread_local bool inited = false;

bool IsInited()
{
    return inited;
}

::lom::Err::Ptr Init()
{
    if (!inited)
    {
        auto err = InitFdEnv();
        if (err)
        {
            return err;
        }
        err = InitSched();
        if (err)
        {
            return err;
        }
        inited = true;
    }
    return nullptr;
}

void MustInit()
{
    auto err = Init();
    if (err)
    {
        Die(Str("lom::fiber::MustInit: init fiber env failed: ").Concat(err->Msg()));
    }
}

void AssertInited()
{
    Assert(inited);
}

}

}
