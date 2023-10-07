#include "internal.h"

namespace lom
{

namespace fiber
{

static const int kFastFdSeqMapSizeMax = 10 * 10000;

static thread_local uint32_t *fast_fd_seq_map = nullptr;
static thread_local std::map<int, uint32_t> slow_fd_seq_map;

static uint32_t &FdSeq(int fd)
{
    return fd >= 0 && fd < kFastFdSeqMapSizeMax ? fast_fd_seq_map[fd] : slow_fd_seq_map[fd];
}

LOM_ERR InitFdEnv()
{
    fast_fd_seq_map = new uint32_t[kFastFdSeqMapSizeMax];
    for (auto i = 0; i < kFastFdSeqMapSizeMax; ++ i)
    {
        fast_fd_seq_map[i] = 0;
    }
    return nullptr;
}

LOM_ERR Fd::Reg(int fd)
{
    AssertInited();

    if (fd_ != -1)
    {
        LOM_RET_ERR("Fd object is already used");
    }

    if (fd < 0)
    {
        LOM_RET_ERR("invalid fd [%d]", fd);
    }

    int flags = 1;
    if (::ioctl(fd, FIONBIO, &flags) == -1)
    {
        LOM_RET_SYS_CALL_ERR("set fd nonblocking failed");
    }

    LOM_RET_ON_ERR(RegRawFdToSched(fd));

    fd_ = fd;
    seq_ = FdSeq(fd_);

    return nullptr;
}

LOM_ERR Fd::Unreg() const
{
    AssertInited();

    if (!Valid())
    {
        LOM_RET_ERR("invalid fd");
    }

    auto err = UnregRawFdFromSched(fd_);
    ++ FdSeq(fd_);
    return err;
}

bool Fd::Valid() const
{
    return fd_ >= 0 && seq_ == FdSeq(fd_);
}

LOM_ERR Fd::Close() const
{
    if (!Valid())
    {
        LOM_RET_ERR("invalid fd");
    }

    LOM_RET_ON_ERR(Unreg());
    if (::close(fd_) == -1)
    {
        LOM_RET_SYS_CALL_ERR("close fd failed");
    }
    return nullptr;
}

void SilentClose(int fd)
{
    int save_errno = errno;
    ::close(fd);
    errno = save_errno;
}

}

}
