#include "internal.h"

namespace lom
{

namespace fiber
{

#define LOM_FIBER_CONN_CHECK_VALID() do {           \
    if (!conn.Valid()) {                            \
        return ::lom::Err::Sprintf("invalid conn"); \
    }                                               \
} while (false)

#define LOM_FIBER_CONN_ON_IO_SYS_CALL_ERR(_r_or_w) do {             \
    Assert(ret == -1 && errno != 0);                                \
    if (errno == ECONNRESET) {                                      \
        return ::lom::SysCallErr::Maker().Make(                     \
            err_code::kConnReset, "connection reset by peer");      \
    }                                                               \
    if (errno == EWOULDBLOCK) {                                     \
        errno = EAGAIN;                                             \
    }                                                               \
    if (errno != EAGAIN && errno != EINTR) {                        \
        return ::lom::SysCallErr::Maker().Make(                     \
            err_code::kSysCallFailed, "syscall error");             \
    }                                                               \
    if (errno == EAGAIN) {                                          \
        WaitingEvents evs;                                          \
        evs.waiting_fds_##_r_or_w##_.emplace_back(conn.RawFd());    \
        SwitchToSchedFiber(std::move(evs));                         \
        if (!conn.Valid()) {                                        \
            return ::lom::SysCallErr::Maker().Make(                 \
                err_code::kClosed, "conn closed by other fiber");   \
        }                                                           \
    }                                                               \
} while (false)

static ::lom::Err::Ptr InternalRead(Conn conn, char *buf, ssize_t sz, ssize_t &rsz)
{
    LOM_FIBER_CONN_CHECK_VALID();

    for (;;)
    {
        auto err = Ctx::Check();
        if (err)
        {
            return err;
        }

        ssize_t ret = read(conn.RawFd(), buf, static_cast<size_t>(sz));
        if (ret >= 0)
        {
            rsz = ret;
            return nullptr;
        }
        LOM_FIBER_CONN_ON_IO_SYS_CALL_ERR(r);
    }
}

static ::lom::Err::Ptr InternalWrite(Conn conn, const char *buf, ssize_t sz, ssize_t &wsz)
{
    LOM_FIBER_CONN_CHECK_VALID();

    auto err = Ctx::Check();
    if (err)
    {
        return err;
    }

    if (sz > 0)
    {
        for (;;)
        {
            ssize_t ret = write(conn.RawFd(), buf, static_cast<size_t>(sz));
            if (ret > 0)
            {
                wsz = ret;
                return nullptr;
            }
            if (ret != 0)
            {
                LOM_FIBER_CONN_ON_IO_SYS_CALL_ERR(w);
            }
            err = Ctx::Check();
            if (err)
            {
                return err;
            }
        }
    }

    return nullptr;
}

static ::lom::Err::Ptr InternalWriteAll(Conn conn, const char *buf, ssize_t sz)
{
    LOM_FIBER_CONN_CHECK_VALID();

    auto err = Ctx::Check();
    if (err)
    {
        return err;
    }

    while (sz > 0)
    {
        ssize_t ret = write(conn.RawFd(), buf, static_cast<size_t>(sz));
        if (ret > 0)
        {
            Assert(ret <= sz);
            buf += ret;
            sz -= ret;
            continue;
        }
        if (ret != 0)
        {
            LOM_FIBER_CONN_ON_IO_SYS_CALL_ERR(w);
        }
        err = Ctx::Check();
        if (err)
        {
            return err;
        }
    }

    return nullptr;
}

::lom::Err::Ptr Conn::Read(char *buf, ssize_t sz, ssize_t &rsz) const
{
    if (sz <= 0)
    {
        return ::lom::Err::Sprintf("invalid buf size");
    }

    return InternalRead(*this, buf, sz, rsz);
}

::lom::Err::Ptr Conn::Write(const char *buf, ssize_t sz, ssize_t &wsz) const
{
    if (sz < 0)
    {
        return ::lom::Err::Sprintf("invalid buf size");
    }

    return InternalWrite(*this, buf, sz, wsz);
}

::lom::Err::Ptr Conn::WriteAll(const char *buf, ssize_t sz) const
{
    if (sz < 0)
    {
        return ::lom::Err::Sprintf("invalid buf size");
    }

    return InternalWriteAll(*this, buf, sz);
}

::lom::Err::Ptr Conn::NewFromRawFd(int fd, Conn &conn)
{
    return conn.Reg(fd);
}

static ::lom::Err::Ptr ConnectStreamSock(
    int socket_family, struct sockaddr *addr, socklen_t addr_len, Conn &conn)
{
    auto err = Ctx::Check();
    if (err)
    {
        return err;
    }

#define LOM_FIBER_CONN_ERR_RETURN(_err_msg, _err_code) do {                         \
    auto _err = ::lom::SysCallErr::Maker().Make((err_code::_err_code), (_err_msg)); \
    SilentClose(conn_sock);                                                         \
    return _err;                                                                    \
} while (false)

    int conn_sock = socket(socket_family, SOCK_STREAM, 0);
    if (conn_sock == -1)
    {
        LOM_FIBER_CONN_ERR_RETURN("create connection socket failed", kSysCallFailed);
    }

    int flags = 1;
    if (ioctl(conn_sock, FIONBIO, &flags) == -1)
    {
        LOM_FIBER_CONN_ERR_RETURN("set connection socket nonblocking failed", kSysCallFailed);
    }

    int ret = connect(conn_sock, addr, addr_len);
    if (ret == -1 && errno != EINPROGRESS)
    {
        LOM_FIBER_CONN_ERR_RETURN("connect failed", kSysCallFailed);
    }

#undef LOM_FIBER_CONN_ERR_RETURN

    err = Conn::NewFromRawFd(conn_sock, conn);
    if (err)
    {
        SilentClose(conn_sock);
        return err;
    }

    if (ret == 0)
    {
        //success
        return nullptr;
    }

    {
        WaitingEvents evs;
        evs.waiting_fds_w_.emplace_back(conn_sock);
        SwitchToSchedFiber(std::move(evs));
    }

#define LOM_FIBER_CONN_ERR_RETURN(_err_msg, _err_code) do {                         \
    auto _err = ::lom::SysCallErr::Maker().Make((err_code::_err_code), (_err_msg)); \
    conn.Close();                                                                   \
    return _err;                                                                    \
} while (false)

    err = Ctx::Check();
    if (err)
    {
        return err;
    }

    int sock_opt_err;
    socklen_t len = sizeof(sock_opt_err);
    if (getsockopt(conn_sock, SOL_SOCKET, SO_ERROR, &sock_opt_err, &len) == -1)
    {
        LOM_FIBER_CONN_ERR_RETURN("getsockopt failed", kSysCallFailed);
    }
    if (sock_opt_err != 0)
    {
        errno = sock_opt_err;
        LOM_FIBER_CONN_ERR_RETURN("connect failed", kSysCallFailed);
    }

    //set tcp nodelay as possible
    int enable = 1;
    setsockopt(conn_sock, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));

#undef LOM_FIBER_CONN_ERR_RETURN

    return nullptr;
}

::lom::Err::Ptr ConnectTCP(const char *ip, uint16_t port, Conn &conn)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (inet_aton(ip, &addr.sin_addr) == 0)
    {
        return ::lom::Err::Sprintf("invalid ip [%s]", ip);
    }
    addr.sin_port = htons(port);

    return ConnectStreamSock(
        AF_INET, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr), conn);
}

::lom::Err::Ptr ConnectUnixSockStream(const char *path, Conn &conn)
{
    struct sockaddr_un addr;
    socklen_t addr_len;
    auto err = PathToUnixSockAddr(path, addr, addr_len);
    if (err)
    {
        return err;
    }

    return ConnectStreamSock(
        AF_UNIX, reinterpret_cast<struct sockaddr *>(&addr), addr_len, conn);
}

::lom::Err::Ptr ConnectUnixSockStreamWithAbstractPath(const Str &path, Conn &conn)
{
    struct sockaddr_un addr;
    socklen_t addr_len;
    auto err = AbstractPathToUnixSockAddr(path, addr, addr_len);
    if (err)
    {
        return err;
    }

    return ConnectStreamSock(
        AF_UNIX, reinterpret_cast<struct sockaddr *>(&addr), addr_len, conn);
}

}

}