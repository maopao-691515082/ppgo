#include "../internal.h"

namespace lom
{

namespace os
{

static LOM_ERR DoStat(bool is_lstat, const Str &path_s, struct stat &st, bool &exists)
{
    const char *path;
    LOM_RET_ON_ERR(path_s.AsCStr(path));

    if ((is_lstat ? ::lstat : ::stat)(path, &st) == 0)
    {
        exists = true;
        return nullptr;
    }
    if (errno == ENOENT)
    {
        exists = false;
        return nullptr;
    }
    LOM_RET_SYS_CALL_ERR("%s `%s` failed", is_lstat ? "lstat" : "stat", path);
}

LOM_ERR FileStat::Stat(const Str &path, FileStat &fst)
{
    return DoStat(false, path, fst.stat_, fst.exists_);
}

LOM_ERR FileStat::LStat(const Str &path, FileStat &fst)
{
    return DoStat(true, path, fst.stat_, fst.exists_);
}

}

}
