#include "../internal.h"

namespace lom
{

namespace os
{

static LOM_ERR DoStat(bool is_lstat, const char *path, struct stat &st, bool &exists)
{
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

LOM_ERR FileStat::Stat(const char *path, FileStat &fst)
{
    return DoStat(false, path, fst.stat_, fst.exists_);
}

LOM_ERR FileStat::LStat(const char *path, FileStat &fst)
{
    return DoStat(true, path, fst.stat_, fst.exists_);
}

}

}
