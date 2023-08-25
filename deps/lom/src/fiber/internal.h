#pragma once

#include "../internal.h"

namespace lom
{

namespace fiber
{

void SilentClose(int fd);

void AssertInited();
::lom::Err::Ptr InitFdEnv();
::lom::Err::Ptr InitSched();

::lom::Err::Ptr RegRawFdToSched(int fd);
::lom::Err::Ptr UnregRawFdFromSched(int fd);

void RegSemToSched(Sem sem, uint64_t value);
::lom::Err::Ptr UnregSemFromSched(Sem sem);
bool IsSemInSched(Sem sem);
uint64_t TryAcquireSem(Sem sem, uint64_t acquire_value);
void RestoreAcquiringSem(Sem sem, uint64_t acquiring_value);
::lom::Err::Ptr ReleaseSem(Sem sem, uint64_t release_value);

struct WaitingEvents
{
    int64_t expire_at_ = -1; //-1表示没设置超时事件
    std::vector<int> waiting_fds_r_, waiting_fds_w_;
    std::vector<Sem> waiting_sems_;

    void Reset()
    {
        expire_at_ = -1;
        waiting_fds_r_.clear();
        waiting_fds_w_.clear();
        waiting_sems_.clear();
    }
};

void SwitchToSchedFiber(const WaitingEvents &evs);

class Fiber
{
    std::function<void ()> run_;

    bool finished_ = false;

    jmp_buf ctx_;
    char *stk_;
    ssize_t stk_sz_;

    int64_t seq_;

    WaitingEvents waiting_evs_;

    static void Start();

    Fiber(std::function<void ()> run, ssize_t stk_sz);

    Fiber(const Fiber&) = delete;
    Fiber& operator=(const Fiber&) = delete;

public:

    ~Fiber();

    bool IsFinished() const
    {
        return finished_;
    }

    jmp_buf *Ctx()
    {
        return &ctx_;
    }

    int64_t Seq() const
    {
        return seq_;
    }

    WaitingEvents &WaitingEvs()
    {
        return waiting_evs_;
    }

    static Fiber *New(std::function<void ()> run, ssize_t stk_sz);

    void Destroy();
};

void RegFiber(Fiber *fiber);
Fiber *GetCurrFiber();
jmp_buf *GetSchedCtx();

::lom::Err::Ptr PathToUnixSockAddr(const char *path, struct sockaddr_un &addr, socklen_t &addr_len);
::lom::Err::Ptr AbstractPathToUnixSockAddr(const Str &path, struct sockaddr_un &addr, socklen_t &addr_len);

}

}
