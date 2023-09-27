#include "../internal.h"

namespace lom
{

namespace os
{

static ::lom::Err::Ptr DoStat(bool is_lstat, const char *path, struct stat &st, bool &exists)
{
    if ((is_lstat ? lstat : stat)(path, &st) == 0)
    {
        exists = true;
        return nullptr;
    }
    if (errno == ENOENT)
    {
        exists = false;
        return nullptr;
    }
    int save_errno = errno;
    return ::lom::Err::Sprintf(
        "syscall `%s` failed, errno = %d", is_lstat ? "lstat" : "stat", save_errno);
}

::lom::Err::Ptr FileStat::Stat(const char *path, FileStat &fst)
{
    return DoStat(false, path, fst.stat_, fst.exists_);
}

::lom::Err::Ptr FileStat::LStat(const char *path, FileStat &fst)
{
    return DoStat(true, path, fst.stat_, fst.exists_);
}

}

}
