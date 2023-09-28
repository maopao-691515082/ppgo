#pragma once

#include "_internal.h"

#include "mem.h"
#include "str.h"
#include "code_pos.h"

namespace lom
{

/*
错误对象的基类，主体是一个虚方法`Msg`和traceback记录机制
使用者可派生自己的错误对象
*/
class Err : public RCObjDyn
{
    std::vector<Str> tb_;

public:

    typedef RCPtr<Err> Ptr;

    void PushTB(CodePos _cp = CodePos())
    {
        tb_.emplace_back(_cp.Str());
    }

    virtual Str Msg() const = 0;

    Str FmtWithTB() const
    {
        Str::Buf b;
        b.Append("Error: ");
        b.Append(Msg());
        b.Append("\n");
        for (auto const &s : tb_)
        {
            b.Append("  from ");
            b.Append(s);
            b.Append("\n");
        }
        return Str(std::move(b));
    }

    //构造最简单的错误对象（只包含一个错误信息），可指定`_cp`为无效位置来禁止记录第一条traceback
    static Ptr FromStr(const Str &msg, CodePos _cp = CodePos());

    //最简错误对象的`Sprintf`版本，因为是可变参数，所以只能禁用traceback，需要的话可手动`FromStr`
    static inline
        __attribute__((always_inline))
        __attribute__((format(gnu_printf, 1, 2)))
    Ptr Sprintf(const char *fmt, ...)
    {
        return FromStr(::lom::Sprintf(fmt, __builtin_va_arg_pack()), CodePos(nullptr, 0, nullptr));
    }
};

/*
通用的系统调用错误，附带类型为`int`的返回值和`errno`
为防止`errno`在创建时被传参改动，这个对象需要用`Maker`辅助类来间接构建
*/
class SysCallErr : public Err
{
    int code_ = -1;
    int errno_;
    Str msg_;

    SysCallErr(int code, int eno, const Str &msg) : code_(code), errno_(eno), msg_(msg)
    {
    }

public:

    class Maker
    {
        int errno_;
    
    public:

        Maker() : errno_(errno)
        {
        }

        Err::Ptr Make(int code, const Str &msg, CodePos _cp = CodePos()) const
        {
            auto err = new SysCallErr(code, errno_, msg);
            err->PushTB(_cp);
            return err;
        }

        Err::Ptr Make(const Str &msg, CodePos _cp = CodePos()) const
        {
            return Make(-1, msg, _cp);
        }

        inline
            __attribute__((always_inline))
            __attribute__((format(gnu_printf, 2, 3)))
        Err::Ptr Sprintf(const char *fmt, ...)
        {
            return Make(::lom::Sprintf(fmt, __builtin_va_arg_pack()), CodePos(nullptr, 0, nullptr));
        }
    };

    int Code() const
    {
        return code_;
    }

    int Errno() const
    {
        return errno_;
    }

    virtual Str Msg() const override
    {
        return ::lom::Sprintf("SysCallErr: %s; <code=%d> <errno=%d>", msg_.CStr(), code_, errno_);
    }
};

}
