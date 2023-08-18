#include "ppgo.h"

int main(int argc, char *argv[])
{
    ppgo::SetArgs(argc, argv);
    auto exc = ppgo::main();
    if (exc)
    {
        auto ftb = exc->FormatWithTB();
        ::ppgo::util::OutputUnexpectedErrMsg(::lom::Sprintf("%s\n", ftb.Data()));
        _exit(2);
    }
}
