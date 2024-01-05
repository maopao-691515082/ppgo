#pragma once

#include "../internal.h"

namespace lom
{

namespace fiber
{

void SilentClose(int fd);

void AssertInited();
LOM_ERR InitFdEnv();
LOM_ERR InitSched();

LOM_ERR RegRawFdToSched(int fd);
LOM_ERR UnregRawFdFromSched(int fd);

void RegSemToSched(Sem sem, uint64_t value);
LOM_ERR UnregSemFromSched(Sem sem);
bool IsSemInSched(Sem sem);
uint64_t TryAcquireSem(Sem sem, uint64_t acquire_value);
void RestoreAcquiringSem(Sem sem, uint64_t acquiring_value);
LOM_ERR ReleaseSem(Sem sem, uint64_t release_value);

struct WaitingEvents
{
    std::vector<int> waiting_fds_r_, waiting_fds_w_;
    std::vector<Sem> waiting_sems_;

    void Reset()
    {
        waiting_fds_r_.clear();
        waiting_fds_w_.clear();
        waiting_sems_.clear();
    }

    bool Empty() const
    {
        return
            waiting_fds_r_.empty() &&
            waiting_fds_w_.empty() &&
            waiting_sems_.empty();
    }
};

void SwitchToSchedFiber(WaitingEvents &&evs);

class Fiber
{
public:

    struct ControlFlowCtx
    {
        int64_t expire_at = -1;
        Sem cancelation_sem;
    };

private:

    std::function<void ()> run_;

    bool finished_ = false;

    jmp_buf ctx_;
    char *stk_;
    ssize_t stk_sz_;

    int64_t seq_;

    WaitingEvents waiting_evs_;

    bool is_worker_ = false;
    Sem cancelation_sem_;

    ControlFlowCtx cf_ctx_;

    static void Start();

    Fiber(std::function<void ()> const &run, ssize_t stk_sz, const Sem *worker_cancelation_sem);

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

    bool IsCanceled() const
    {
        return is_worker_ && !cancelation_sem_.Valid();
    }

    ControlFlowCtx &CFCtx()
    {
        return cf_ctx_;
    }

    bool IsCFTimeout() const
    {
        return cf_ctx_.expire_at >= 0 && cf_ctx_.expire_at <= NowMS();
    }

    LOM_ERR CheckCtx() const
    {
        if (IsCanceled())
        {
            LOM_RET_SYS_CALL_ERR_WITH_CODE(err_code::kCanceled, "canceled");
        }
        if (IsCFTimeout())
        {
            LOM_RET_SYS_CALL_ERR_WITH_CODE(err_code::kTimeout, "timeout");
        }
        return nullptr;
    }

    bool IsWorker() const
    {
        return is_worker_;
    }

    Sem CancelationSem() const
    {
        return cancelation_sem_;
    }

    static Fiber *New(std::function<void ()> const &run, ssize_t stk_sz, const Sem *worker_cancelation_sem);

    void Destroy();
};

void RegFiber(Fiber *fiber);
Fiber *GetCurrFiber();
jmp_buf *GetSchedCtx();

LOM_ERR PathToUnixSockAddr(const Str &path, struct sockaddr_un &addr, socklen_t &addr_len);
LOM_ERR AbstractPathToUnixSockAddr(const Str &path, struct sockaddr_un &addr, socklen_t &addr_len);

}

}
