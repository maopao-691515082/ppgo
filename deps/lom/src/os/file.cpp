#include "../internal.h"

namespace lom
{

namespace os
{

::lom::Err::Ptr File::Lock() const
{
    return flock(fd_, LOCK_EX) == -1 ?
        ::lom::SysCallErr::Maker().Sprintf("`flock` failed") :
        nullptr;
}

::lom::Err::Ptr File::TryLock(bool &ok) const
{
    ok = flock(fd_, LOCK_EX | LOCK_NB) == 0;
    return ok || errno == EWOULDBLOCK ?
        nullptr :
        ::lom::SysCallErr::Maker().Sprintf("`flock` failed");
}

::lom::Err::Ptr File::Unlock() const
{
    return flock(fd_, LOCK_UN) == -1 ?
        ::lom::SysCallErr::Maker().Sprintf("`flock` failed") :
        nullptr;
}

::lom::Err::Ptr File::Open(const char *path, File::Ptr &fp, const char *mode, int perm_bits)
{
    int open_flags = 0;
    const char *m = mode;
    char mm = *m;
    ++ m;
    bool is_rw = *m == '+';
    if (is_rw)
    {
        open_flags = O_RDWR;
        ++ m;
    }

    if (mm == 'r')
    {
        if (!is_rw)
        {
            open_flags = O_RDONLY;
        }
    }
    else if (mm == 'w' || mm == 'a')
    {
        if (!is_rw)
        {
            open_flags = O_WRONLY;
        }
        open_flags |= O_CREAT | (mm == 'w' ? O_TRUNC : O_APPEND);
    }
    else
    {
        return ::lom::Err::Sprintf("invalid mode: %s", mode);
    }

    for (; *m && *m != ','; ++ m)
    {
        if (*m == 'e')
        {
            open_flags |= O_CLOEXEC;
            continue;
        }
        if (*m == 'x' && mm != 'r')
        {
            open_flags |= O_EXCL;
            continue;
        }
    }

    int fd = ::open(path, open_flags, perm_bits);
    if (fd == -1)
    {
        return ::lom::SysCallErr::Maker().Sprintf("open file `%s` with mode `%s` failed", path, mode);
    }

    fp = File::Ptr(new File(fd));
    return nullptr;
}

}

}
