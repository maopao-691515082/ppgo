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

LOM_ERR Init()
{
    if (!inited)
    {
        LOM_RET_ON_ERR(InitFdEnv());
        LOM_RET_ON_ERR(InitSched());
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
