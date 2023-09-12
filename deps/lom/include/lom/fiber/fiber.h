#pragma once

#include "../_internal.h"

#include "_err.h"
#include "_fd.h"
#include "_conn.h"
#include "_listener.h"
#include "_sem.h"

namespace lom
{

namespace fiber
{

//栈大小范围[128KB, 8MB]
static const ssize_t
    kStkSizeMin = 128 * 1024,
    kStkSizeMax = 8 * 1024 * 1024;

bool IsInited();
::lom::Err::Ptr Init();
void MustInit();    //init or die

/*
创建新的fiber
`run`为入口函数
*/
struct CreateOptions
{
    //指定栈大小，不在范围则调整至边界值
    ssize_t stk_sz  = kStkSizeMin;

    //指定创建worker fiber，worker在创建者结束时会收到canceled信号
    bool is_worker  = false;
};
void Create(std::function<void ()> run, CreateOptions opts = CreateOptions());
//创建worker的快捷接口，会无视`opts`的`is_worker`
void CreateWorker(std::function<void ()> run, CreateOptions opts = CreateOptions());

//开始运行，除非出现内部错误（一般就是系统异常导致epoll_wait失败），否则永远不退出
::lom::Err::Ptr Run();

/*
在新的控制流上下文执行函数，当前控制流会被压栈
控制流上下文中的超时是叠加生效的，例如在一个10秒超时的流程中开启一个20秒超时的流程，
则后者的超时时刻会被调整至和前者相同
*/
class Ctx
{
    int64_t timeout_ms_;

public:

    Ctx(int64_t timeout_ms = -1) : timeout_ms_(timeout_ms)
    {
    }

    //`Call`的返回值是透传f的返回
    ::lom::Err::Ptr Call(std::function<::lom::Err::Ptr ()> f) const;

    //检查当前控制流上下文，如有取消或超时则以错误形式返回
    static ::lom::Err::Ptr Check();
};

/*
当前fiber睡眠一段时间，受控制流上下文影响，可能提前结束睡眠
参数`ms`若<=0则只做错误检测，即相当于`Ctx::Check()`
*/
::lom::Err::Ptr SleepMS(int64_t ms);

//切回调度器，注意`Yield`不受控制流上下文的影响
void Yield();

}

}
