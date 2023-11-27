#include "internal.h"

namespace lom
{

namespace fiber
{

LOM_ERR Listener::Accept(Conn &conn) const
{
    if (!Valid())
    {
        LOM_RET_ERR("invalid listener");
    }

#define LOM_FIBER_LISTENER_ERR_RETURN(_err_msg, _err_code) do {         \
    LOM_RET_SYS_CALL_ERR_WITH_CODE((err_code::_err_code), (_err_msg));  \
} while (false)

    for (;;)
    {
        LOM_RET_ON_ERR(Ctx::Check());

        int fd = ::accept(RawFd(), nullptr, nullptr);
        if (fd >= 0)
        {
            auto err = Conn::NewFromRawFd(fd, conn);
            if (err)
            {
                SilentClose(fd);
                return err;
            }

            //set tcp nodelay as possible
            int enable = 1;
            ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));

            return nullptr;
        }
        Assert(fd == -1 && errno != 0);
        if (errno == EWOULDBLOCK)
        {
            errno = EAGAIN;
        }
        if (errno != EAGAIN && errno != EINTR)
        {
            LOM_FIBER_LISTENER_ERR_RETURN("listener accept failed", kSysCallFailed);
        }
        if (errno == EAGAIN)
        {
            WaitingEvents evs;
            evs.waiting_fds_r_.emplace_back(RawFd());
            SwitchToSchedFiber(std::move(evs));
            if (!Valid())
            {
                LOM_FIBER_LISTENER_ERR_RETURN("listener closed by other fiber", kClosed);
            }
        }
    }

#undef LOM_FIBER_LISTENER_ERR_RETURN

}

class ServeWorker
{
    size_t idx_;
    std::function<void (size_t)> init_worker_;
    std::function<void (Conn)> work_with_conn_;
    std::function<void (LOM_ERR)> err_log_;

    std::mutex lock_;
    std::vector<int> conn_fds_;

    void ThreadMain()
    {
        ::lom::fiber::MustInit();

        if (init_worker_)
        {
            init_worker_(idx_);
        }

        ::lom::fiber::Create([this] () {
            for (;;)
            {
                std::vector<int> conn_fds;
                {
                    std::lock_guard<std::mutex> lg(lock_);
                    conn_fds = std::move(conn_fds_);
                    conn_fds_.clear();
                }

                if (conn_fds.empty())
                {
                    ::lom::fiber::SleepMS(1);
                    continue;
                }

                for (int fd : conn_fds)
                {
                    Conn conn;
                    auto err = Conn::NewFromRawFd(fd, conn);
                    if (err)
                    {
                        if (err_log_)
                        {
                            err->PushTB();
                            err_log_(err);
                        }
                        ::close(fd);
                        continue;
                    }

                    ::lom::fiber::Create([work_with_conn = work_with_conn_, conn] () {
                        work_with_conn(conn);
                    });

                    ::lom::fiber::Yield();
                }
            }
        });

        ::lom::fiber::Run();
    }

public:

    ServeWorker(
        size_t idx, std::function<void (size_t)> init_worker, std::function<void (Conn)> work_with_conn,
        std::function<void (LOM_ERR)> err_log
    ) :
        idx_(idx), init_worker_(init_worker), work_with_conn_(work_with_conn), err_log_(err_log)
    {
        std::thread(&ServeWorker::ThreadMain, this).detach();
    }

    void DeliverConnFd(int fd)
    {
        std::lock_guard<std::mutex> lg(lock_);
        conn_fds_.emplace_back(fd);
    }
};

LOM_ERR Listener::Serve(
    size_t worker_count, std::function<void (Conn)> work_with_conn,
    std::function<void (LOM_ERR)> err_log, std::function<void (size_t)> init_worker) const
{
    if (worker_count > kWorkerCountMax)
    {
        worker_count = kWorkerCountMax;
    }
    std::vector<ServeWorker *> workers(worker_count);
    for (size_t i = 0; i < worker_count; ++i)
    {
        workers[i] = new ServeWorker(i, init_worker, work_with_conn, err_log);
    }

    size_t next_worker_idx = 0;
    for (;;)
    {
        Conn conn;
        auto err = Accept(conn);
        if (err)
        {
            auto sys_call_err = dynamic_cast<::lom::SysCallErr *>(err.get());
            if (sys_call_err && sys_call_err->Code() == ::lom::fiber::err_code::kClosed)
            {
                return err;
            }
            if (err_log)
            {
                err->PushTB();
                err_log(err);
            }
            ::lom::fiber::SleepMS(1);
            continue;
        }

        if (worker_count == 0)
        {
            ::lom::fiber::Create([work_with_conn, conn] () {
                work_with_conn(conn);
            });
            continue;
        }

        err = conn.Unreg();
        if (err)
        {
            if (err_log)
            {
                err->PushTB();
                err_log(err);
            }
            conn.Close();
            continue;
        }

        workers.at(next_worker_idx)->DeliverConnFd(conn.RawFd());
        next_worker_idx = (next_worker_idx + 1) % workers.size();
    }
}

LOM_ERR Listener::NewFromRawFd(int fd, Listener &listener)
{
    return listener.Reg(fd);
}

static LOM_ERR ListenStream(
    int socket_family, struct sockaddr *addr, socklen_t addr_len, const Str &addr_desc,
    Listener &listener, int listen_fd = -1)
{
    if (listen_fd < 0)
    {
        listen_fd = ::socket(socket_family, SOCK_STREAM, 0);
        if (listen_fd == -1)
        {
            LOM_RET_SYS_CALL_ERR_WITH_CODE(err_code::kSysCallFailed, "create listen socket failed");
        }
    }

#define LOM_FIBER_LISTENER_ERR_RETURN(_err_msg) do {                                    \
    auto _err = ::lom::SysCallErr::Maker().Make(err_code::kSysCallFailed, (_err_msg));  \
    SilentClose(listen_fd);                                                             \
    return _err;                                                                        \
} while (false)

    if (::bind(listen_fd, addr, addr_len) == -1)
    {
        LOM_FIBER_LISTENER_ERR_RETURN(Sprintf("bind listener to addr `%s` failed", addr_desc.CStr()));
    }

    if (::listen(listen_fd, 1024) == -1)
    {
        LOM_FIBER_LISTENER_ERR_RETURN("listen failed");
    }

#undef LOM_FIBER_LISTENER_ERR_RETURN

    auto err = Listener::NewFromRawFd(listen_fd, listener);
    if (err)
    {
        SilentClose(listen_fd);
        return err;
    }

    return nullptr;
}

LOM_ERR ListenTCP(uint16_t port, Listener &listener)
{
    int listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        LOM_RET_SYS_CALL_ERR_WITH_CODE(err_code::kSysCallFailed, "create listen socket failed");
    }

    int reuseaddr_on = 1;
    if (::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on)) == -1)
    {
        auto err = ::lom::SysCallErr::Maker().Make(
            err_code::kSysCallFailed, "set listen socket reuse-addr failed");
        SilentClose(listen_fd);
        return err;
    }

    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(port);

    return ListenStream(
        AF_INET, reinterpret_cast<struct sockaddr *>(&listen_addr), sizeof(listen_addr),
        Sprintf("tcp4[:%u]", static_cast<unsigned int>(port)), listener, listen_fd);
}

LOM_ERR ListenUnixSockStream(const Str &path, Listener &listener)
{
    struct sockaddr_un addr;
    socklen_t addr_len;
    LOM_RET_ON_ERR(PathToUnixSockAddr(path, addr, addr_len));
    return ListenStream(
        AF_UNIX, reinterpret_cast<struct sockaddr *>(&addr), addr_len,
        Sprintf("unix[%s]", path.CStr()), listener);
}

LOM_ERR ListenUnixSockStreamWithAbstractPath(const Str &path, Listener &listener)
{
    struct sockaddr_un addr;
    socklen_t addr_len;
    LOM_RET_ON_ERR(AbstractPathToUnixSockAddr(path, addr, addr_len));
    return ListenStream(
        AF_UNIX, reinterpret_cast<struct sockaddr *>(&addr), addr_len,
        Sprintf("unix-abstract[%s]", path.Repr().CStr()), listener);
}

}

}
