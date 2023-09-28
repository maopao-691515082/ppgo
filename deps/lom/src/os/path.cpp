#include "../internal.h"

namespace lom
{

namespace os
{

static ::lom::Err::Ptr PathMake(const char *path, GoSlice<::lom::Str> &paths)
{
    ::lom::Str abs_path;
    if (*path == '/')
    {
        abs_path = path;
    }
    else
    {
        char *cwd = ::getcwd(nullptr, 0);
        if (cwd == nullptr)
        {
            return ::lom::SysCallErr::Maker().Sprintf("`getcwd` failed");
        }
        if (*cwd != '/')
        {
            return ::lom::Err::Sprintf("invalid `getcwd` result: `%s`", cwd);
        }
        abs_path = ::lom::Sprintf("%s/%s", cwd, path);
        free(cwd);
    }
    Assert(abs_path.HasPrefix("/"));
    auto parts = abs_path.Slice().Split("/");
    paths = paths.Nil();
    for (ssize_t i = 0; i < parts.Len(); ++ i)
    {
        auto part = parts.At(i);
        if (part == "" || part == ".")
        {
            continue;
        }
        if (part == ".." && paths.Len() > 0)
        {
            paths = paths.Slice(0, paths.Len() - 1);
            continue;
        }
        paths = paths.Append(part);
    }
    return nullptr;
}

Path::Path(const char *path)
{
    GoSlice<::lom::Str> paths;
    ::lom::Err::Ptr err = PathMake(path, paths);
    Assert(!err);
    paths_ = paths;
}

::lom::Str Path::Str() const
{
    if (paths_.Len() == 0)
    {
        return "/";
    }

    ::lom::Str::Buf b;
    for (ssize_t i = 0; i < paths_.Len(); ++ i)
    {
        b.Append("/");
        b.Append(paths_.At(i));
    }
    return ::lom::Str(std::move(b));
}

::lom::Err::Ptr Path::Make(const char *path_str, Path &path)
{
    GoSlice<::lom::Str> paths;
    auto err = PathMake(path_str, paths);
    if (err)
    {
        return err;
    }
    path = Path(paths);
    return nullptr;
}

}

}
