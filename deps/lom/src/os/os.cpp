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

LOM_ERR RemoveTree(const char *path)
{
    FileStat fst;
    LOM_RET_ON_ERR(FileStat::LStat(path, fst));
    if (fst.IsDir())
    {
        GoSlice<Str> names;
        LOM_RET_ON_ERR(ListDir(path, names));
        for (auto const &name : names)
        {
            LOM_RET_ON_ERR(RemoveTree(Sprintf("%s/%s", path, name.CStr()).CStr()));
        }
    }
    if (::remove(path) == -1)
    {
        LOM_RET_SYS_CALL_ERR("remove `%s` failed", path);
    }
    return nullptr;
}

LOM_ERR Rename(const char *old_path, const char *new_path)
{
    if (::rename(old_path, new_path) == -1)
    {
        LOM_RET_SYS_CALL_ERR("rename `%s` to `%s` failed", old_path, new_path);
    }
    return nullptr;
}

}

}
