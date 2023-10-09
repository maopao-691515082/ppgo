#include "../internal.h"

namespace lom
{

namespace os
{

LOM_ERR ListDir(const char *path, GoSlice<Str> &ret_names)
{
    DIR *d = ::opendir(path);
    if (d == nullptr)
    {
        LOM_RET_SYS_CALL_ERR("`opendir` `%s` failed", path);
    }
    Defer defer_close_d([&] () {
        ::closedir(d);
    });

    GoSlice<Str> names;
    for (;;)
    {
        errno = 0;
        struct dirent *di = ::readdir(d);
        if (di == nullptr)
        {
            if (errno != 0)
            {
                LOM_RET_SYS_CALL_ERR("`readdir` `%s` failed", path);
            }
            break;
        }
        Str name = di->d_name;
        if (name != "." && name != "..")
        {
            names = names.Append(std::move(name));
        }
    }

    names.Sort();
    ret_names = names;
    return nullptr;
}

}

}
