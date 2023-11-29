#include "../internal.h"

namespace lom
{

namespace os
{

::lom::Str NormPath(const Str &path)
{
    GoSlice<::lom::Str> paths;
    int prefix_dd = 0;
    for (const auto &part : path.Slice().Split("/"))
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

    bool is_abs_path = path.HasPrefix("/");
    if (is_abs_path)
    {
        prefix_dd = 0;
    }
    if (prefix_dd > 0)
    {
        paths.Reverse();
        for (; prefix_dd > 0; -- prefix_dd)
        {
            paths = paths.Append("..");
        }
        paths.Reverse();
    }
    if (paths.Len() == 0)
    {
        return is_abs_path ? "/" : ".";
    }

    ::lom::Str::Buf b;
    if (is_abs_path)
    {
        b.Append("/");
    }
    b.Append(paths.At(0));
    for (ssize_t i = 1; i < paths.Len(); ++ i)
    {
        b.Append("/");
        b.Append(paths.At(i));
    }
    return Str(std::move(b));
}

LOM_ERR AbsPath(const Str &path, Str &abs_path)
{
    Str p = path;
    if (!p.HasPrefix("/"))
    {
        char *cwd_cstr = ::getcwd(nullptr, 0);
        if (cwd_cstr == nullptr)
        {
            LOM_RET_SYS_CALL_ERR("`getcwd` failed");
        }
        Str cwd(cwd_cstr);
        free(cwd_cstr);

        if (!cwd.HasPrefix("/"))
        {
            LOM_RET_ERR("invalid `getcwd` result: `%s`", cwd.CStr());
        }

        p = cwd.Concat("/").Concat(p);
    }
    abs_path = NormPath(p);
    return nullptr;
}

}

}
