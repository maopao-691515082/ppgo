#include "internal.h"

namespace lom
{

namespace fiber
{

#define LOM_FIBER_CONN_CHECK_VALID() do {   \
    if (!conn.Valid()) {                    \
        LOM_RET_ERR("invalid conn");        \
    }                                       \
} while (false)

#define LOM_FIBER_CONN_ON_IO_SYS_CALL_ERR(_r_or_w) do {             \
    Assert(ret == -1 && errno != 0);                                \
    if (errno == ECONNRESET) {                                      \
        LOM_RET_SYS_CALL_ERR_WITH_CODE(                             \
            err_code::kConnReset, "connection reset by peer");      \
    }                                                               \
    if (errno == EWOULDBLOCK) {                                     \
        errno = EAGAIN;                                             \
    }                                                               \
    if (errno != EAGAIN && errno != EINTR) {                        \
        LOM_RET_SYS_CALL_ERR_WITH_CODE(                             \
            err_code::kSysCallFailed, "conn read/write error");     \
    }                                                               \
    if (errno == EAGAIN) {                                          \
        WaitingEvents evs;                                          \
        evs.waiting_fds_##_r_or_w##_.emplace_back(conn.RawFd());    \
        SwitchToSchedFiber(std::move(evs));                         \
        if (!conn.Valid()) {                                        \
            LOM_RET_SYS_CALL_ERR_WITH_CODE(                         \
                err_code::kClosed, "conn closed by other fiber");   \
        }                                                           \
    }                                                               \
} while (false)

static LOM_ERR InternalRead(Conn conn, char *buf, ssize_t sz, ssize_t &rsz)
{
    LOM_FIBER_CONN_CHECK_VALID();

    for (;;)
    {
        LOM_RET_ON_ERR(Ctx::Check());

        ssize_t ret = ::read(conn.RawFd(), buf, static_cast<size_t>(sz));
        if (ret >= 0)
        {
            rsz = ret;
            return nullptr;
        }
        LOM_FIBER_CONN_ON_IO_SYS_CALL_ERR(r);
    }
}

static LOM_ERR InternalWrite(Conn conn, const char *buf, ssize_t sz, ssize_t &wsz)
{
    LOM_FIBER_CONN_CHECK_VALID();

    LOM_RET_ON_ERR(Ctx::Check());

    if (sz > 0)
    {
        for (;;)
        {
            ssize_t ret = ::write(conn.RawFd(), buf, static_cast<size_t>(sz));
            if (ret > 0)
            {
                wsz = ret;
                return nullptr;
            }
            if (ret != 0)
            {
                LOM_FIBER_CONN_ON_IO_SYS_CALL_ERR(w);
            }
            LOM_RET_ON_ERR(Ctx::Check());
        }
    }

    return nullptr;
}

static LOM_ERR InternalWriteAll(Conn conn, const char *buf, ssize_t sz)
{
    LOM_FIBER_CONN_CHECK_VALID();

    LOM_RET_ON_ERR(Ctx::Check());

    while (sz > 0)
    {
        ssize_t ret = ::write(conn.RawFd(), buf, static_cast<size_t>(sz));
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
        LOM_RET_ON_ERR(Ctx::Check());
    }

    return nullptr;
}

LOM_ERR Conn::Read(char *buf, ssize_t sz, ssize_t &rsz) const
{
    if (sz <= 0)
    {
        LOM_RET_ERR("invalid buf size `%zd`", sz);
    }

    return InternalRead(*this, buf, sz, rsz);
}

LOM_ERR Conn::Write(const char *buf, ssize_t sz, ssize_t &wsz) const
{
    if (sz < 0)
    {
        LOM_RET_ERR("invalid buf size `%zd`", sz);
    }

    return InternalWrite(*this, buf, sz, wsz);
}

LOM_ERR Conn::WriteAll(const char *buf, ssize_t sz) const
{
    if (sz < 0)
    {
        LOM_RET_ERR("invalid buf size `%zd`", sz);
    }

    return InternalWriteAll(*this, buf, sz);
}

LOM_ERR Conn::NewFromRawFd(int fd, Conn &conn)
{
    return conn.Reg(fd);
}

static LOM_ERR ConnectStreamSock(
    int socket_family, struct sockaddr *addr, socklen_t addr_len, Conn &conn)
{
    LOM_RET_ON_ERR(Ctx::Check());

#define LOM_FIBER_CONN_ERR_RETURN(_err_msg, _err_code) do {                         \
    auto _err = ::lom::SysCallErr::Maker().Make((err_code::_err_code), (_err_msg)); \
    SilentClose(conn_sock);                                                         \
    return _err;                                                                    \
} while (false)

    int conn_sock = ::socket(socket_family, SOCK_STREAM, 0);
    if (conn_sock == -1)
    {
        LOM_FIBER_CONN_ERR_RETURN("create connection socket failed", kSysCallFailed);
    }

    int flags = 1;
    if (::ioctl(conn_sock, FIONBIO, &flags) == -1)
    {
        LOM_FIBER_CONN_ERR_RETURN("set connection socket nonblocking failed", kSysCallFailed);
    }

    int ret = ::connect(conn_sock, addr, addr_len);
    if (ret == -1 && errno != EINPROGRESS)
    {
        LOM_FIBER_CONN_ERR_RETURN("connect failed", kSysCallFailed);
    }

#undef LOM_FIBER_CONN_ERR_RETURN

    auto err = Conn::NewFromRawFd(conn_sock, conn);
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

    LOM_RET_ON_ERR(Ctx::Check());

    int sock_opt_err;
    socklen_t len = sizeof(sock_opt_err);
    if (::getsockopt(conn_sock, SOL_SOCKET, SO_ERROR, &sock_opt_err, &len) == -1)
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

LOM_ERR ConnectTCP(const Str &ipv4, uint16_t port, Conn &conn)
{
    const char *ip;
    LOM_RET_ON_ERR(ipv4.AsCStr(ip));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (inet_aton(ip, &addr.sin_addr) == 0)
    {
        LOM_RET_ERR("invalid ip [%s]", ip);
    }
    addr.sin_port = htons(port);

    return ConnectStreamSock(
        AF_INET, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr), conn);
}

LOM_ERR ConnectUnixSockStream(const Str &path, Conn &conn)
{
    struct sockaddr_un addr;
    socklen_t addr_len;
    LOM_RET_ON_ERR(PathToUnixSockAddr(path, addr, addr_len));
    return ConnectStreamSock(
        AF_UNIX, reinterpret_cast<struct sockaddr *>(&addr), addr_len, conn);
}

LOM_ERR ConnectUnixSockStreamWithAbstractPath(const Str &path, Conn &conn)
{
    struct sockaddr_un addr;
    socklen_t addr_len;
    LOM_RET_ON_ERR(AbstractPathToUnixSockAddr(path, addr, addr_len));
    return ConnectStreamSock(
        AF_UNIX, reinterpret_cast<struct sockaddr *>(&addr), addr_len, conn);
}

}

}