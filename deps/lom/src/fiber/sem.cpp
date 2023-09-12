#include "internal.h"

namespace lom
{

namespace fiber
{

::lom::Err::Ptr Sem::Destroy() const
{
    return UnregSemFromSched(*this);
}

bool Sem::Valid() const
{
    return seq_ >= 0 && IsSemInSched(*this);
}

::lom::Err::Ptr Sem::Acquire(uint64_t acquire_value) const
{
    if (!Valid())
    {
        return ::lom::Err::Sprintf("invalid sem");
    }

    /*
    循环`acquire`直到需要的`acquire_value`都申请完成，或到sem被销毁、超时失败等情况
    需要注意的是如果超时或取消，则需返还`done_value`
    */

    uint64_t done_value = 0;
    while (acquire_value > done_value)
    {
        auto err = Ctx::Check();
        if (err)
        {
            if (done_value > 0)
            {
                //已经申请了一部分了，返还
                RestoreAcquiringSem(*this, done_value);
            }
            return err;
        }

        done_value += TryAcquireSem(*this, acquire_value - done_value);
        if (done_value == acquire_value)
        {
            return nullptr;
        }

        //没有扣完，需要阻塞等待
        Assert(done_value < acquire_value);

        WaitingEvents evs;
        evs.waiting_sems_.emplace_back(*this);
        SwitchToSchedFiber(evs);

        if (!Valid())
        {
            return ::lom::SysCallErr::Maker().Make(err_code::kClosed, "sem destroyed by another fiber");
        }
    }

    return nullptr;
}

::lom::Err::Ptr Sem::Release(uint64_t release_value) const
{
    return ReleaseSem(*this, release_value);
}

::lom::Err::Ptr Sem::New(uint64_t value, Sem &sem)
{
    static thread_local int64_t next_sem_seq = 1;

    sem.seq_ = next_sem_seq;
    ++ next_sem_seq;
    RegSemToSched(sem, value);
    return nullptr;
}

}

}
