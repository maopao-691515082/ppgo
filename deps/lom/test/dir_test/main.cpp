#include <iostream>

#include "../../include/lom.h"

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++ i)
    {
        ::lom::GoSlice<::lom::Str> names;
        auto err = ::lom::os::ListDir(argv[i], names);
        if (err)
        {
            LOM_DIE("list dir failed: %s", err->Msg().CStr());
        }
        std::cout << "`" << argv[i] << "`" << std::endl;
        for (ssize_t j = 0; j < names.Len(); ++ j)
        {
            std::cout << "|---`" << names.At(j).CStr() << "`" << std::endl;
        }
    }
}
