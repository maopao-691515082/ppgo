#include "../internal.h"

namespace lom
{

namespace os
{

::lom::Err::Ptr MakeDirs(const Path &path, int perm_bits)
{
    auto path_str = path.Str();
    FileStat fst;
    auto err = FileStat::Stat(path_str.CStr(), fst);
    if (err)
    {
        return err;
    }
    if (fst.IsDir())
    {
        return nullptr;
    }
    if (fst.Exists())
    {
        return ::lom::Err::Sprintf("path `%s` exists and is not a dir", path_str.CStr());
    }
    err = MakeDirs(path.Dir(), perm_bits);
    if (err)
    {
        return err;
    }
    if (mkdir(path_str.CStr(), perm_bits) == -1)
    {
        return ::lom::SysCallErr::Maker().Sprintf("mkdir `%s` failed", path_str.CStr());
    }
    return nullptr;
}

}

}
