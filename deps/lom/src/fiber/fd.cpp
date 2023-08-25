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

::lom::Err::Ptr InitFdEnv()
{
    fast_fd_seq_map = new uint32_t[kFastFdSeqMapSizeMax];
    for (auto i = 0; i < kFastFdSeqMapSizeMax; ++ i)
    {
        fast_fd_seq_map[i] = 0;
    }
    return nullptr;
}

::lom::Err::Ptr Fd::Reg(int fd)
{
    AssertInited();

    if (fd_ != -1)
    {
        return ::lom::Err::Sprintf("Fd object is already used");
    }

    if (fd < 0)
    {
        return ::lom::Err::Sprintf("invalid fd [%d]", fd);
    }

    int flags = 1;
    if (ioctl(fd, FIONBIO, &flags) == -1)
    {
        return ::lom::Err::Sprintf("set fd nonblocking failed");
    }

    auto err = RegRawFdToSched(fd);
    if (err)
    {
        return err;
    }

    fd_ = fd;
    seq_ = FdSeq(fd_);

    return nullptr;
}

::lom::Err::Ptr Fd::Unreg() const
{
    AssertInited();

    if (!Valid())
    {
        return ::lom::Err::Sprintf("fd is invalid");
    }

    auto err = UnregRawFdFromSched(fd_);
    ++ FdSeq(fd_);
    return err;
}

bool Fd::Valid() const
{
    return fd_ >= 0 && seq_ == FdSeq(fd_);
}

::lom::Err::Ptr Fd::Close() const
{
    if (!Valid())
    {
        return ::lom::Err::Sprintf("fd is invalid");
    }

    auto err = Unreg();
    if (close(fd_) == -1)
    {
        auto unreg_err_msg = err ? err->Msg().Concat(" & ") : "";
        err = ::lom::Err::Sprintf("%sclose fd failed", unreg_err_msg.CStr());
    }
    return err;
}

void SilentClose(int fd)
{
    int save_errno = errno;
    close(fd);
    errno = save_errno;
}

}

}
