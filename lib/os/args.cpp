#include "ppgo.h"

#pragma ppgo define-THIS_MOD

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

#pragma ppgo undef-THIS_MOD
