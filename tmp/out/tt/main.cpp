#include "ppgo.h"

int main(int argc, char *argv[])
{
#ifdef PPGO_SINGLE_THREADING_MODE

    fprintf(stderr, "\e[;33mPPGO: RUNNING AT SINGLE THREADING MODE\e[0m\n\n");

#endif

    ppgo::SetArgs(argc, argv);
    auto exc = ppgo::main();
    if (exc)
    {
        auto ftb = exc->FormatWithTB();
        fprintf(stderr, "%s\n", ftb.Data());
        exit(2);
    }
}
