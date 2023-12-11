#include "internal.h"

namespace lom
{

namespace fiber
{

static thread_local int64_t next_fiber_seq = 1;

//temporary for initing new fiber
static thread_local Fiber *initing_fiber;
static thread_local ucontext_t fiber_init_ret_uctx;

Fiber::Fiber(std::function<void ()> run, ssize_t stk_sz, const Sem *worker_cancelation_sem) :
    run_(run), finished_(false), stk_sz_(stk_sz), is_worker_(worker_cancelation_sem != nullptr)
{
    stk_ = new char[stk_sz_];

    seq_ = next_fiber_seq;
    ++ next_fiber_seq;

    if (is_worker_)
    {
        cancelation_sem_ = *worker_cancelation_sem;
    }

    ucontext_t uctx;
    Assert(getcontext(&uctx) == 0);
    uctx.uc_stack.ss_sp = stk_;
    uctx.uc_stack.ss_size = stk_sz_;
    uctx.uc_link = nullptr;
    makecontext(&uctx, Fiber::Start, 0);
    initing_fiber = this;
    Assert(swapcontext(&fiber_init_ret_uctx, &uctx) == 0);
}

Fiber::~Fiber()
{
    delete[] stk_;
}

void Fiber::Start()
{
    //init ctx
    if (setjmp(initing_fiber->ctx_) == 0)
    {
        //swap and never back here
        ucontext_t tmp_uctx;
        Assert(swapcontext(&tmp_uctx, &fiber_init_ret_uctx) == 0);
    }

    for (;;)    //future: for recycling
    {
        Fiber *curr_fiber = GetCurrFiber();

        curr_fiber->cf_ctx_.expire_at = -1;
        auto err = Sem::New(0, curr_fiber->cf_ctx_.cancelation_sem);
        Assert(!err);

        curr_fiber->run_();

        LOM_DISCARD(curr_fiber->cf_ctx_.cancelation_sem.Destroy());

        curr_fiber->finished_ = true;

        curr_fiber->run_ = [] () {};
        curr_fiber->waiting_evs_.Reset();

        //to sched
        if (setjmp(curr_fiber->ctx_) == 0)
        {
            longjmp(*GetSchedCtx(), 1);
        }
    }
}

Fiber *Fiber::New(std::function<void ()> run, ssize_t stk_sz, const Sem *worker_cancelation_sem)
{
    return new Fiber(run, stk_sz, worker_cancelation_sem);
}

void Fiber::Destroy()
{
    delete this;
}

void Create(std::function<void ()> run, CreateOptions opts)
{
    AssertInited();

    if (opts.stk_sz < kStkSizeMin)
    {
        opts.stk_sz = kStkSizeMin;
    }
    if (opts.stk_sz > kStkSizeMax)
    {
        opts.stk_sz = kStkSizeMax;
    }

    const Sem *worker_cancelation_sem = nullptr;
    if (opts.is_worker)
    {
        Fiber *curr_fiber = GetCurrFiber();
        if (curr_fiber != nullptr)
        {
            worker_cancelation_sem = &curr_fiber->CFCtx().cancelation_sem;
        }
    }

    RegFiber(Fiber::New(run, opts.stk_sz, worker_cancelation_sem));
}

void CreateWorker(std::function<void ()> run, CreateOptions opts)
{
    opts.is_worker = true;
    Create(run, opts);
}

LOM_ERR Ctx::Call(std::function<LOM_ERR ()> f) const
{
    AssertInited();

    Fiber *curr_fiber = GetCurrFiber();
    Assert(curr_fiber != nullptr);
    Fiber::ControlFlowCtx &cf_ctx = curr_fiber->CFCtx();

    Fiber::ControlFlowCtx cf_ctx_old = cf_ctx;

    int64_t expire_at = timeout_ms_ < 0 ? -1 : NowMS() + timeout_ms_;
    if (expire_at >= 0 && (cf_ctx.expire_at < 0 || expire_at < cf_ctx.expire_at))
    {
        cf_ctx.expire_at = expire_at;
    }

    auto err = Sem::New(0, cf_ctx.cancelation_sem);
    Assert(!err);

    Defer defer_recover_cf_ctx([&] () {
        LOM_DISCARD(cf_ctx.cancelation_sem.Destroy());
        cf_ctx = cf_ctx_old;
    });

    return f();
}

}

}
