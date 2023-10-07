#include "internal.h"

namespace lom
{

namespace fiber
{

static thread_local Fiber *curr_fiber = nullptr;
static thread_local jmp_buf sched_ctx;

typedef std::map<int64_t /*fiber_seq*/, Fiber *> Fibers;

class ReadyFibers
{
    std::vector<Fiber *> sorted_fibers_;
    Fibers fibers_;

public:

    bool Empty() const
    {
        return sorted_fibers_.empty();
    }

    void Add(Fiber *fiber)
    {
        if (fibers_.count(fiber->Seq()) == 0)
        {
            sorted_fibers_.emplace_back(fiber);
            fibers_[fiber->Seq()] = fiber;
        }
    }

    std::vector<Fiber *> Move()
    {
        std::vector<Fiber *> sfs = std::move(sorted_fibers_);
        sorted_fibers_.clear();
        fibers_.clear();
        return sfs;
    }
};
static thread_local ReadyFibers ready_fibers;

static thread_local std::map<int64_t /*expire_at*/, Fibers> expire_waiting_fibers;

struct FdWaitingFibers
{
    Fibers r_, w_;  //fibers waiting reading and writing
};
static thread_local std::map<int /*raw_fd*/, FdWaitingFibers> io_waiting_fibers;

struct SemInfo
{
    uint64_t    value_;
    int64_t     acquiring_fiber_seq_ = -1;
    uint64_t    acquiring_value_ = 0;
    Fibers      fibers_;
};
static thread_local std::map<Sem, SemInfo> sem_infos;

static void WakeUpFibers(const Fibers &fibers_to_wake_up)
{
    //for each, add ready_fibers and remove from waiting queues
    for (auto fiber_iter = fibers_to_wake_up.begin(); fiber_iter != fibers_to_wake_up.end(); ++ fiber_iter)
    {
        int64_t fiber_seq = fiber_iter->first;
        Fiber *fiber = fiber_iter->second;

        ready_fibers.Add(fiber);

        auto expire_at = fiber->CFCtx().expire_at;
        if (expire_at >= 0)
        {
            auto iter = expire_waiting_fibers.find(expire_at);
            if (iter != expire_waiting_fibers.end())
            {
                iter->second.erase(fiber_seq);
            }
        }

        WaitingEvents &evs = fiber->WaitingEvs();

#define LOM_FIBER_SCHED_CLEAR_FIBER_FROM_IO_WAITING_FIBERS(_r_or_w) do {    \
    for (auto fd : evs.waiting_fds_##_r_or_w##_) {                          \
        io_waiting_fibers[fd]._r_or_w##_.erase(fiber_seq);                  \
    }                                                                       \
    evs.waiting_fds_##_r_or_w##_.clear();                                   \
} while (false)

        LOM_FIBER_SCHED_CLEAR_FIBER_FROM_IO_WAITING_FIBERS(r);
        LOM_FIBER_SCHED_CLEAR_FIBER_FROM_IO_WAITING_FIBERS(w);

#undef LOM_FIBER_SCHED_CLEAR_FIBER_FROM_IO_WAITING_FIBERS

        for (Sem sem: evs.waiting_sems_)
        {
            auto iter = sem_infos.find(sem);
            if (iter != sem_infos.end())
            {
                iter->second.fibers_.erase(fiber_seq);
            }
        }
        evs.waiting_sems_.clear();
    }
}

static void RegCurrFiberWaitingEvs(WaitingEvents &&evs)
{
    auto expire_at = curr_fiber->CFCtx().expire_at;
    if (expire_at >= 0)
    {
        expire_waiting_fibers[expire_at][curr_fiber->Seq()] = curr_fiber;
    }

    WaitingEvents &curr_evs = curr_fiber->WaitingEvs();

#define LOM_FIBER_SCHED_REGISTER_CURR_FIBER_IO_WAITING_FIBERS(_r_or_w) do {             \
    if (evs.waiting_fds_##_r_or_w##_.size() > 0) {                                      \
        curr_evs.waiting_fds_##_r_or_w##_ = std::move(evs.waiting_fds_##_r_or_w##_);    \
        for (auto fd : curr_evs.waiting_fds_##_r_or_w##_) {                             \
            auto fd_waiting_fibers_iter = io_waiting_fibers.find(fd);                   \
            Assert(fd_waiting_fibers_iter != io_waiting_fibers.end());                  \
            fd_waiting_fibers_iter->second._r_or_w##_[curr_fiber->Seq()] = curr_fiber;  \
        }                                                                               \
    }                                                                                   \
} while (false)

    LOM_FIBER_SCHED_REGISTER_CURR_FIBER_IO_WAITING_FIBERS(r);
    LOM_FIBER_SCHED_REGISTER_CURR_FIBER_IO_WAITING_FIBERS(w);

#undef LOM_FIBER_SCHED_REGISTER_CURR_FIBER_IO_WAITING_FIBERS

    if (evs.waiting_sems_.size() > 0)
    {
        curr_evs.waiting_sems_ = std::move(evs.waiting_sems_);
        for (Sem sem: curr_evs.waiting_sems_)
        {
            auto sem_infos_iter = sem_infos.find(sem);
            Assert(sem_infos_iter != sem_infos.end());
            sem_infos_iter->second.fibers_[curr_fiber->Seq()] = curr_fiber;
        }
    }

    if (curr_fiber->IsWorker())
    {
        auto sem = curr_fiber->CancelationSem();
        curr_evs.waiting_sems_.emplace_back(sem);
        auto sem_infos_iter = sem_infos.find(sem);
        Assert(sem_infos_iter != sem_infos.end());
        sem_infos_iter->second.fibers_[curr_fiber->Seq()] = curr_fiber;
    }
}

static thread_local int ep_fd = -1;

LOM_ERR InitSched()
{
    ep_fd = ::epoll_create1(0);
    if (ep_fd == -1)
    {
        LOM_RET_SYS_CALL_ERR_WITH_CODE(err_code::kSysCallFailed, "create epoll fd failed");
    }

    if (::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        LOM_RET_SYS_CALL_ERR_WITH_CODE(err_code::kSysCallFailed, "ignore SIGPIPE failed");
    }

    return nullptr;
}

jmp_buf *GetSchedCtx()
{
    return &sched_ctx;
}

void RegFiber(Fiber *fiber)
{
    ready_fibers.Add(fiber);
}

Fiber *GetCurrFiber()
{
    return curr_fiber;
}

LOM_ERR RegRawFdToSched(int fd)
{
    if (io_waiting_fibers.find(fd) != io_waiting_fibers.end())
    {
        LOM_RET_ERR("fd [%d] is already registered", fd);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.fd = fd;
    if (::epoll_ctl(ep_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        LOM_RET_SYS_CALL_ERR_WITH_CODE(err_code::kSysCallFailed, "epoll_ctl EPOLL_CTL_ADD failed");
    }

    io_waiting_fibers.insert(std::make_pair(fd, FdWaitingFibers()));
    return nullptr;
}

LOM_ERR UnregRawFdFromSched(int fd)
{
    auto fd_waiting_fibers_iter = io_waiting_fibers.find(fd);
    Assert(fd_waiting_fibers_iter != io_waiting_fibers.end());

    FdWaitingFibers &fd_waiting_fibers = fd_waiting_fibers_iter->second;

#define LOM_FIBER_SCHED_WAKE_UP_ALL_FD_WAITING_FIBERS(_r_or_w) do {     \
    Fibers fibers_to_wake_up(std::move(fd_waiting_fibers._r_or_w##_));  \
    fd_waiting_fibers._r_or_w##_.clear();                               \
    WakeUpFibers(fibers_to_wake_up);                                    \
} while (false)

    LOM_FIBER_SCHED_WAKE_UP_ALL_FD_WAITING_FIBERS(r);
    LOM_FIBER_SCHED_WAKE_UP_ALL_FD_WAITING_FIBERS(w);

#undef LOM_FIBER_SCHED_WAKE_UP_ALL_FD_WAITING_FIBERS

    io_waiting_fibers.erase(fd_waiting_fibers_iter);

    struct epoll_event ev;  //must specify an ev struct in old-version epoll
    if (::epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, &ev) == -1)
    {
        LOM_RET_SYS_CALL_ERR_WITH_CODE(err_code::kSysCallFailed, "epoll_ctl EPOLL_CTL_DEL failed");
    }

    return nullptr;
}

void RegSemToSched(Sem sem, uint64_t value)
{
    AssertInited();
    Assert(sem_infos.count(sem) == 0);
    sem_infos[sem].value_ = value;
}

LOM_ERR UnregSemFromSched(Sem sem)
{
    AssertInited();

    auto sem_infos_iter = sem_infos.find(sem);
    if (sem_infos_iter == sem_infos.end())
    {
        LOM_RET_ERR("invalid sem");
    }

    SemInfo &sem_info = sem_infos_iter->second;

    Fibers fibers_to_wake_up(std::move(sem_info.fibers_));
    sem_info.fibers_.clear();
    WakeUpFibers(fibers_to_wake_up);

    sem_infos.erase(sem_infos_iter);
    return nullptr;
}

bool IsSemInSched(Sem sem)
{
    return sem_infos.count(sem) != 0;
}

uint64_t TryAcquireSem(Sem sem, uint64_t acquire_value)
{
    auto sem_infos_iter = sem_infos.find(sem);
    Assert(sem_infos_iter != sem_infos.end() && acquire_value > 0);

    SemInfo &sem_info = sem_infos_iter->second;

    if (sem_info.acquiring_fiber_seq_ < 0)
    {
        //no fiber acquiring

        Assert(sem_info.acquiring_value_ == 0);

        if (sem_info.value_ >= acquire_value)
        {
            //enough value, success
            sem_info.value_ -= acquire_value;
            return acquire_value;
        }

        //not enough value, turn to acquiring
        uint64_t done_value = sem_info.value_;
        sem_info.value_ = 0;
        sem_info.acquiring_fiber_seq_ = curr_fiber->Seq();
        sem_info.acquiring_value_ = done_value;
        return done_value;
    }

    if (curr_fiber->Seq() != sem_info.acquiring_fiber_seq_)
    {
        //some other fiber is acquiring, can't acquire
        return 0;
    }

    //continue acquiring

    if (sem_info.value_ >= acquire_value)
    {
        //enough, success
        sem_info.value_ -= acquire_value;
        sem_info.acquiring_fiber_seq_ = -1;
        sem_info.acquiring_value_ = 0;
        return acquire_value;
    }

    //not enough, acquire more
    uint64_t done_value = sem_info.value_;
    sem_info.value_ = 0;
    Assert(kUInt64Max - sem_info.acquiring_value_ >= done_value);
    sem_info.acquiring_value_ += done_value;
    return done_value;
}

void RestoreAcquiringSem(Sem sem, uint64_t acquiring_value)
{
    auto sem_infos_iter = sem_infos.find(sem);
    Assert(sem_infos_iter != sem_infos.end());

    SemInfo &sem_info = sem_infos_iter->second;

    Assert(
        sem_info.acquiring_fiber_seq_ == curr_fiber->Seq() &&
        sem_info.acquiring_value_ == acquiring_value);

    Assert(kUInt64Max - sem_info.value_ >= acquiring_value);
    sem_info.value_ += acquiring_value;
    sem_info.acquiring_fiber_seq_ = -1;
    sem_info.acquiring_value_ = 0;

    Fibers fibers_to_wake_up(std::move(sem_info.fibers_));
    sem_info.fibers_.clear();
    WakeUpFibers(fibers_to_wake_up);
}

LOM_ERR ReleaseSem(Sem sem, uint64_t release_value)
{
    auto sem_infos_iter = sem_infos.find(sem);
    if (sem_infos_iter == sem_infos.end())
    {
        LOM_RET_ERR("invalid sem");
    }

    SemInfo &sem_info = sem_infos_iter->second;

    Assert(kUInt64Max - sem_info.value_ >= sem_info.acquiring_value_);
    if (kUInt64Max - sem_info.value_ - sem_info.acquiring_value_ < release_value)
    {
        LOM_RET_ERR("sem value overflow");
    }
    sem_info.value_ += release_value;

    Fibers fibers_to_wake_up(std::move(sem_info.fibers_));
    sem_info.fibers_.clear();
    WakeUpFibers(fibers_to_wake_up);

    return nullptr;
}

static void DoSwitchToSchedFiber()
{
    if (setjmp(*curr_fiber->Ctx()) == 0)
    {
        longjmp(sched_ctx, 1);
    }
}

void SwitchToSchedFiber(WaitingEvents &&evs)
{
    Assert(curr_fiber != nullptr);

    //there must be some waiting-events
    Assert(curr_fiber->CFCtx().expire_at >= 0 || !evs.Empty());
    RegCurrFiberWaitingEvs(std::move(evs));
    DoSwitchToSchedFiber();
}

void Yield()
{
    AssertInited();

    //ready at once
    Assert(curr_fiber != nullptr);
    ready_fibers.Add(curr_fiber);
    DoSwitchToSchedFiber();
}

LOM_ERR SleepMS(int64_t ms)
{
    AssertInited();

    if (ms < 0)
    {
        ms = 0;
    }

    Assert(curr_fiber != nullptr);

    LOM_RET_ON_ERR(curr_fiber->CheckCtx());

    int64_t expire_at = NowMS() + ms;
    int64_t &cf_ctx_expire_at = curr_fiber->CFCtx().expire_at;
    int64_t old_expire_at = cf_ctx_expire_at;

    if (cf_ctx_expire_at < 0 || expire_at < cf_ctx_expire_at)
    {
        cf_ctx_expire_at = expire_at;
    }

    {
        WaitingEvents evs;
        SwitchToSchedFiber(std::move(evs));
    }

    cf_ctx_expire_at = old_expire_at;

    LOM_RET_ON_ERR(curr_fiber->CheckCtx());

    return nullptr;
}

LOM_ERR Ctx::Check()
{
    AssertInited();

    Assert(curr_fiber != nullptr);
    return curr_fiber->CheckCtx();
}

LOM_ERR Run()
{
    AssertInited();
    for (;;)
    {
        if (!ready_fibers.Empty())
        {
            std::vector<Fiber *> running_fibers = ready_fibers.Move();
            for (auto fiber : running_fibers)
            {
                curr_fiber = fiber;
                if (setjmp(sched_ctx) == 0)
                {
                    longjmp(*curr_fiber->Ctx(), 1);
                }
                curr_fiber = nullptr;

                if (fiber->IsFinished())
                {
                    fiber->Destroy();
                }
            }
        }

        //check expiring
        {
            int64_t now = NowMS();
            while (!expire_waiting_fibers.empty())
            {
                auto iter = expire_waiting_fibers.begin();
                int64_t expire_at = iter->first;
                if (expire_at > now)
                {
                    break;
                }
                //pop and wake up
                Fibers fibers_to_wake_up(std::move(iter->second));
                expire_waiting_fibers.erase(iter);
                WakeUpFibers(fibers_to_wake_up);
            }
        }

        //check io ev
        {
            int ep_wait_timeout = ready_fibers.Empty() ? 100 : 0;
            if (!expire_waiting_fibers.empty())
            {
                int64_t now = NowMS();
                int64_t min_expire_at = expire_waiting_fibers.begin()->first;
                ep_wait_timeout = (
                    min_expire_at > now ? std::min(ep_wait_timeout, (int)(min_expire_at - now)) : 0);
            }

            static const int kEpollEvCountMax = 1024;
            struct epoll_event evs[kEpollEvCountMax];
            int ev_count = ::epoll_wait(ep_fd, evs, kEpollEvCountMax, ep_wait_timeout);
            if (ev_count == -1)
            {
                if (errno != EINTR)
                {
                    LOM_RET_SYS_CALL_ERR_WITH_CODE(err_code::kSysCallFailed, "epoll_wait failed");
                }
                ev_count = 0;
            }

            if (ev_count > 0)
            {
                for (int i = 0; i < ev_count; ++ i)
                {
                    const struct epoll_event &ev = evs[i];
                    int fd = ev.data.fd;
                    auto fd_waiting_fibers_iter = io_waiting_fibers.find(fd);
                    if (fd_waiting_fibers_iter == io_waiting_fibers.end())
                    {
                        //deletion of this fd was failed, ignore
                        continue;
                    }
                    FdWaitingFibers &fd_waiting_fibers = fd_waiting_fibers_iter->second;

#define LOM_FIBER_SCHED_WAKE_UP_BY_EVENT(_ev, _r_or_w) do {                 \
    if (ev.events & (EPOLL##_ev | EPOLLERR | EPOLLHUP)) {                   \
        Fibers fibers_to_wake_up(std::move(fd_waiting_fibers._r_or_w##_));  \
        fd_waiting_fibers._r_or_w##_.clear();                               \
        WakeUpFibers(fibers_to_wake_up);                                    \
    }                                                                       \
} while (false)

                    LOM_FIBER_SCHED_WAKE_UP_BY_EVENT(IN, r);
                    LOM_FIBER_SCHED_WAKE_UP_BY_EVENT(OUT, w);

#undef LOM_FIBER_SCHED_WAKE_UP_BY_EVENT

                }
            }
        }
    }
}

}

}
