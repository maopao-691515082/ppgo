#include "../internal.h"

namespace lom
{

namespace os
{

LOM_ERR File::Lock(int fd)
{
    if (flock(fd, LOCK_EX) == -1)
    {
        LOM_RET_SYS_CALL_ERR("`flock` failed");
    }
    return nullptr;
}

LOM_ERR File::TryLock(int fd, bool &ok)
{
    ok = flock(fd, LOCK_EX | LOCK_NB) == 0;
    if (ok || errno == EWOULDBLOCK)
    {
        return nullptr;
    }
    LOM_RET_SYS_CALL_ERR("`flock` failed");
}

LOM_ERR File::Unlock(int fd)
{
    if (flock(fd, LOCK_UN) == -1)
    {
        LOM_RET_SYS_CALL_ERR("`flock` failed");
    }
    return nullptr;
}

LOM_ERR File::Seek(int fd, ssize_t off, SeekWhence whence, ssize_t *new_off)
{
    int lseek_whence;
    switch (whence)
    {
        case kSeekSet:
        {
            lseek_whence = SEEK_SET;
            break;
        }
        case kSeekCur:
        {
            lseek_whence = SEEK_CUR;
            break;
        }
        case kSeekEnd:
        {
            lseek_whence = SEEK_END;
            break;
        }
        default:
        {
            LOM_RET_ERR("invalid `whence` value `%d`", static_cast<int>(whence));
        }
    }
    off_t ret = ::lseek(fd, static_cast<off_t>(off), lseek_whence);
    if (ret == static_cast<off_t>(-1))
    {
        LOM_RET_SYS_CALL_ERR("`lseek` failed");
    }
    if (new_off)
    {
        *new_off = ret;
    }
    return nullptr;
}

LOM_ERR File::Read(int fd, char *buf, ssize_t sz, ssize_t &rsz)
{
    if (sz <= 0)
    {
        LOM_RET_ERR("invalid buf size `%zd`", sz);
    }

    for (;;)
    {
        ssize_t ret = ::read(fd, buf, static_cast<size_t>(sz));
        if (ret >= 0)
        {
            rsz = ret;
            return nullptr;
        }
        if (errno != EINTR)
        {
            LOM_RET_SYS_CALL_ERR("`read` failed");
        }
    }
}

LOM_ERR File::Write(int fd, const char *buf, ssize_t sz, ssize_t &wsz)
{
    if (sz < 0)
    {
        LOM_RET_ERR("invalid buf size `%zd`", sz);
    }

    for (;;)
    {
        ssize_t ret = ::write(fd, buf, static_cast<size_t>(sz));
        if (ret >= 0)
        {
            wsz = ret;
            return nullptr;
        }
        if (errno != EINTR)
        {
            LOM_RET_SYS_CALL_ERR("`write` failed");
        }
    }
}

LOM_ERR File::ReadFull(int fd, char *buf, ssize_t sz, ssize_t *ret_rsz)
{
    ssize_t total_rsz = 0;
    for (;;)
    {
        ssize_t rsz;
        LOM_RET_ON_ERR(Read(fd, buf + total_rsz, sz - total_rsz, rsz));
        total_rsz += rsz;
        if (rsz == 0 || total_rsz == sz)
        {
            break;
        }
        Assert(total_rsz < sz);
    }
    if (ret_rsz)
    {
        *ret_rsz = total_rsz;
    }
    return total_rsz == sz ? nullptr : ::lom::io::UnexpectedEOF();
}

LOM_ERR File::WriteAll(int fd, const char *buf, ssize_t sz)
{
    for (;;)
    {
        ssize_t wsz;
        LOM_RET_ON_ERR(Write(fd, buf, sz, wsz));
        Assert(wsz <= sz);
        buf += wsz;
        sz -= wsz;
        if (sz == 0)
        {
            return nullptr;
        }
    }
}

LOM_ERR File::PRead(int fd, ssize_t off, char *buf, ssize_t sz, ssize_t &rsz)
{
    if (sz <= 0)
    {
        LOM_RET_ERR("invalid buf size `%zd`", sz);
    }

    for (;;)
    {
        ssize_t ret = ::pread(fd, buf, static_cast<size_t>(sz), static_cast<off_t>(off));
        if (ret >= 0)
        {
            rsz = ret;
            return nullptr;
        }
        if (errno != EINTR)
        {
            LOM_RET_SYS_CALL_ERR("`pread` failed");
        }
    }
}

LOM_ERR File::PWrite(int fd, ssize_t off, const char *buf, ssize_t sz, ssize_t &wsz)
{
    if (sz < 0)
    {
        LOM_RET_ERR("invalid buf size `%zd`", sz);
    }

    for (;;)
    {
        ssize_t ret = ::pwrite(fd, buf, static_cast<size_t>(sz), static_cast<off_t>(off));
        if (ret >= 0)
        {
            wsz = ret;
            return nullptr;
        }
        if (errno != EINTR)
        {
            LOM_RET_SYS_CALL_ERR("`pwrite` failed");
        }
    }
}

LOM_ERR File::PReadFull(int fd, ssize_t off, char *buf, ssize_t sz, ssize_t *ret_rsz)
{
    ssize_t total_rsz = 0;
    for (;;)
    {
        ssize_t rsz;
        LOM_RET_ON_ERR(PRead(fd, off + total_rsz, buf + total_rsz, sz - total_rsz, rsz));
        total_rsz += rsz;
        if (rsz == 0 || total_rsz == sz)
        {
            break;
        }
        Assert(total_rsz < sz);
    }
    if (ret_rsz)
    {
        *ret_rsz = total_rsz;
    }
    return total_rsz == sz ? nullptr : ::lom::io::UnexpectedEOF();
}

LOM_ERR File::PWriteAll(int fd, ssize_t off, const char *buf, ssize_t sz)
{
    for (;;)
    {
        ssize_t wsz;
        LOM_RET_ON_ERR(PWrite(fd, off, buf, sz, wsz));
        Assert(wsz <= sz);
        off += wsz;
        buf += wsz;
        sz -= wsz;
        if (sz == 0)
        {
            return nullptr;
        }
    }
}

LOM_ERR File::Open(const char *path, File::Ptr &fp, const char *mode, int perm_bits)
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
        LOM_RET_ERR("invalid mode: %s", mode);
    }

    bool disable_cloexec = false;
    for (; *m && *m != ','; ++ m)
    {
        if (*m == 'E')
        {
            disable_cloexec = true;
            continue;
        }
        if (*m == 'x' && mm != 'r')
        {
            open_flags |= O_EXCL;
            continue;
        }
    }
    if (!disable_cloexec)
    {
        open_flags |= O_CLOEXEC;
    }

    int fd = ::open(path, open_flags, perm_bits);
    if (fd == -1)
    {
        LOM_RET_SYS_CALL_ERR("open file `%s` with mode `%s` failed", path, mode);
    }

    fp = File::Ptr(new File(fd));
    return nullptr;
}

}

}
