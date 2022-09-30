#include "ppgo.h"

#ifdef PPGO_THIS_MOD
#error macro PPGO_THIS_MOD redefined
#endif
#define PPGO_THIS_MOD modDD30

namespace ppgo
{

namespace PPGO_THIS_MOD
{

}

void SetArgs(int argc, char *argv[])
{
    for (int i = 0; i < argc; ++ i)
    {
        ::ppgo::PPGO_THIS_MOD::gv_args.Append(::ppgo::tp_string(argv[i]));
    }
}

}

#undef PPGO_THIS_MOD
