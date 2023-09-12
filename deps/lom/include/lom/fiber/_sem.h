#pragma once

namespace lom
{

namespace fiber
{

class Sem
{
    int64_t seq_ = -1;

public:

    bool operator<(const Sem &other) const
    {
        return seq_ < other.seq_;
    }

    ::lom::Err::Ptr Destroy() const;

    bool Valid() const;

    /*
    获取和释放信号量
        获取的错误只有信号量无效、超时、取消或信号量被其他fiber销毁等
        释放的错误只有溢出
        一般在使用者可控的情况下不需要判断返回
    */
    ::lom::Err::Ptr Acquire(uint64_t acquire_value = 1) const;
    ::lom::Err::Ptr Release(uint64_t release_value = 1) const;

    //指定初值创建信号量
    static ::lom::Err::Ptr New(uint64_t value, Sem &sem);
};

}

}
