#include "../internal.h"

namespace lom
{

namespace io
{

namespace fds
{

LOM_ERR Read(int fd, char *buf, ssize_t sz, ssize_t &rsz)
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

LOM_ERR Write(int fd, const char *buf, ssize_t sz, ssize_t &wsz)
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

LOM_ERR ReadFull(int fd, char *buf, ssize_t sz, ssize_t *ret_rsz)
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

LOM_ERR WriteAll(int fd, const char *buf, ssize_t sz)
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

}

}

}
