#include "../internal.h"

namespace lom
{

namespace os
{

LOM_ERR MakeDirs(const Path &path, int perm_bits)
{
    auto path_str = path.Str();
    FileStat fst;
    LOM_RET_ON_ERR(FileStat::Stat(path_str.CStr(), fst));
    if (fst.IsDir())
    {
        return nullptr;
    }
    if (fst.Exists())
    {
        LOM_RET_ERR("non-dir path `%s`", path_str.CStr());
    }
    LOM_RET_ON_ERR(MakeDirs(path.Dir(), perm_bits));
    if (::mkdir(path_str.CStr(), perm_bits) == -1)
    {
        LOM_RET_SYS_CALL_ERR("make dir `%s` failed", path_str.CStr());
    }
    return nullptr;
}

}

}
