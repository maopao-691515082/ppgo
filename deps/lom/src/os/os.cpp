#include "../internal.h"

namespace lom
{

namespace os
{

LOM_ERR MakeDirs(const Str &path_in, int perm_bits)
{
    Str path;
    LOM_RET_ON_ERR(AbsPath(path_in, path));
    Assert(path.HasPrefix("/"));
    for (ssize_t idx = 1; idx <= path.Len(); ++ idx)
    {
        while (idx < path.Len() && path.Get(idx) != '/')
        {
            ++ idx;
        }

        auto p = path.SubStr(0, idx);
        FileStat fst;
        LOM_RET_ON_ERR(FileStat::Stat(p, fst));
        if (fst.IsDir())
        {
            continue;
        }
        if (fst.Exists())
        {
            LOM_RET_ERR("non-dir path `%s`", p.CStr());
        }
        if (::mkdir(p.CStr(), perm_bits) == -1)
        {
            LOM_RET_SYS_CALL_ERR("make dir `%s` failed", p.CStr());
        }
    }
    return nullptr;
}

LOM_ERR RemoveTree(const Str &path_in)
{
    Str path;
    LOM_RET_ON_ERR(AbsPath(path_in, path));
    FileStat fst;
    LOM_RET_ON_ERR(FileStat::LStat(path, fst));
    if (!fst.Exists())
    {
        return nullptr;
    }
    if (fst.IsDir())
    {
        GoSlice<Str> names;
        LOM_RET_ON_ERR(ListDir(path, names));
        for (auto const &name : names)
        {
            LOM_RET_ON_ERR(RemoveTree(Sprintf("%s/%s", path.CStr(), name.CStr())));
        }
    }
    if (::remove(path.CStr()) == -1)
    {
        LOM_RET_SYS_CALL_ERR("remove `%s` failed", path.CStr());
    }
    return nullptr;
}

LOM_ERR Rename(const Str &old_path_s, const Str &new_path_s)
{
    const char *old_path, *new_path;
    LOM_RET_ON_ERR(old_path_s.AsCStr(old_path));
    LOM_RET_ON_ERR(new_path_s.AsCStr(new_path));
    if (::rename(old_path, new_path) == -1)
    {
        LOM_RET_SYS_CALL_ERR("rename `%s` to `%s` failed", old_path, new_path);
    }
    return nullptr;
}

}

}
