#include "../internal.h"

namespace lom
{

namespace os
{

Path::Path(const char *path)
{
    ::lom::Str abs_path;
    if (*path == '/')
    {
        abs_path = path;
    }
    else
    {
        char *cwd = getcwd(nullptr, 0);
        abs_path = ::lom::Sprintf("%s/%s", cwd, path);
        free(cwd);
    }
    Assert(abs_path.HasPrefix("/"));
    auto parts = abs_path.Slice().Split("/");
    for (ssize_t i = 0; i < parts.Len(); ++ i)
    {
        auto part = parts.At(i);
        if (part == "" || part == ".")
        {
            continue;
        }
        if (part == ".." && paths_.Len() > 0)
        {
            paths_ = paths_.Slice(0, paths_.Len() - 1);
            continue;
        }
        paths_ = paths_.Append(part);
    }
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

::lom::Err::Ptr Path::MakeDirs() const
{
    return ::lom::Err::Sprintf("todo");
}

}

}
