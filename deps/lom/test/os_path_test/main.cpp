#include <iostream>

#include "../../include/lom.h"

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++ i)
    {
        ::lom::os::Path path(argv[i]);
        std::cout << "[" << path.Str().CStr() << "]" << std::endl;
    }
}
