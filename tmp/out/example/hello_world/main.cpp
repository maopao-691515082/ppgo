#include "ppgo.h"

int main(int argc, char *argv[])
{
    std::tuple<> ret;
    auto exc = ppgo::main(ret);
    if (exc)
    {
        auto ftb = exc->FormatWithTB();
        fprintf(stderr, "%s\n", ftb.Data());
        exit(2);
    }
}
