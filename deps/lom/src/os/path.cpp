#include "../internal.h"

namespace lom
{

namespace os
{

static LOM_ERR PathMake(const char *p, GoSlice<::lom::Str> &paths, bool keep_relative = false)
{
    ::lom::Str path;
    if (*p == '/' || keep_relative)
    {
        path = p;
    }
    else
    {
        char *cwd = ::getcwd(nullptr, 0);
        if (cwd == nullptr)
        {
            LOM_RET_SYS_CALL_ERR("`getcwd` failed");
        }
        if (*cwd != '/')
        {
            LOM_RET_ERR("invalid `getcwd` result: `%s`", cwd);
        }
        path = ::lom::Sprintf("%s/%s", cwd, p);
        free(cwd);
    }
    bool is_abs_path = path.HasPrefix("/");
    if (!keep_relative)
    {
        Assert(is_abs_path);
    }
    auto parts = path.Slice().Split("/");
    paths = paths.Nil();
    ssize_t prefix_dd = 0;
    for (auto const &part : parts)
    {
        if (part == "" || part == ".")
        {
            continue;
        }
        if (part == "..")
        {
            if (paths.Len() == 0)
            {
                ++ prefix_dd;
            }
            else
            {
                paths = paths.Slice(0, paths.Len() - 1);
            }
            continue;
        }
        paths = paths.Append(part);
    }
    if (!is_abs_path && prefix_dd > 0)
    {
        paths.Reverse();
        for (ssize_t i = 0; i < prefix_dd; ++ i)
        {
            paths = paths.Append("..");
        }
        paths.Reverse();
    }
    return nullptr;
}

::lom::Str NormPath(const Str &path_s)
{
    const char *path;
    {
        LOM_ERR err = path_s.AsCStr(path);
        Assert(!err);
    }

    GoSlice<::lom::Str> paths;
    {
        LOM_ERR err = PathMake(path, paths, true);
        Assert(!err);
    }

    if (paths.Len() == 0)
    {
        return *path == '/' ? "/" : ".";
    }

    ::lom::Str::Buf b;
    for (ssize_t i = 0; i < paths.Len(); ++ i)
    {
        if (i != 0 || *path == '/')
        {
            b.Append("/");
        }
        b.Append(paths.At(i));
    }
    return ::lom::Str(std::move(b));
}

Path::Path(const ::lom::Str &path_s)
{
    const char *path;
    {
        LOM_ERR err = path_s.AsCStr(path);
        Assert(!err);
    }

    GoSlice<::lom::Str> paths;
    {
        LOM_ERR err = PathMake(path, paths);
        Assert(!err);
    }
    paths_ = paths;
}

::lom::Str Path::Str() const
{
    if (paths_.Len() == 0)
    {
        return "/";
    }

    ::lom::Str::Buf b;
    for (auto const &p : paths_)
    {
        b.Append("/");
        b.Append(p);
    }
    return ::lom::Str(std::move(b));
}

LOM_ERR Path::Make(const ::lom::Str &path_s, Path &path)
{
    const char *path_cs;
    LOM_RET_ON_ERR(path_s.AsCStr(path_cs));

    GoSlice<::lom::Str> paths;
    LOM_RET_ON_ERR(PathMake(path_cs, paths));
    path = Path(paths);
    return nullptr;
}

}

}
