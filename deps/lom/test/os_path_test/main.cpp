#include <iostream>

#include "../../include/lom.h"

int main(int argc, char *argv[])
{
    if (argc < 2 || !(strcmp(argv[1], "abs") == 0 || strcmp(argv[1], "norm") == 0))
    {
        std::cerr << "Usage: " << argv[0] << " {abs|norm} <PATH>..." << std::endl;
        exit(1);
    }

    for (int i = 2; i < argc; ++ i)
    {
        if (strcmp(argv[1], "abs") == 0)
        {
            ::lom::os::Path path(argv[i]);
            std::cout << "[" << path.Str().CStr() << "]" << std::endl;
        }
        else
        {
            std::cout << "[" << ::lom::os::NormPath(argv[i]).CStr() << "]" << std::endl;
        }
    }
}
