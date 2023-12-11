#pragma once

#include "_internal.h"

#include "str.h"
#include "code_pos.h"

namespace lom
{

/*
错误对象的基类，主体是一个虚方法`Msg`和traceback记录机制
使用者可派生自己的错误对象
*/
class Err
{
    std::vector<Str> tb_;

public:

    virtual ~Err() = default;

    void PushTB(CodePos _cp = CodePos())
    {
        tb_.emplace_back(_cp.Str());
    }

    virtual Str Msg() const = 0;

    Str FmtWithTB() const;

    //判断是否为系统调用错误，可指定`code`或`eno`来精确判断具体的错误码或errno
    bool IsSysCallErr(::std::optional<int> code = ::std::nullopt, ::std::optional<int> eno = ::std::nullopt);

    //构造最简单的错误对象（只包含一个错误信息），可指定`_cp`为无效位置来禁止记录第一条traceback
    static LOM_ERR FromStr(const Str &msg, CodePos _cp = CodePos());
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

        LOM_ERR Make(int code, const Str &msg, CodePos _cp = CodePos()) const
        {
            auto err = LOM_ERR(new SysCallErr(code, errno_, msg));
            err->PushTB(_cp);
            return err;
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

//这些宏可以简化使用

#define LOM_RET_ERR(_fmt, ...) do {                                                     \
    return ::lom::Err::FromStr(::lom::Sprintf(_fmt, ##__VA_ARGS__), ::lom::CodePos());  \
} while (false)

#define LOM_RET_SYS_CALL_ERR_WITH_CODE(_code, _fmt, ...) do {                                               \
    return ::lom::SysCallErr::Maker().Make((_code), ::lom::Sprintf(_fmt, ##__VA_ARGS__), ::lom::CodePos()); \
} while (false)

#define LOM_RET_SYS_CALL_ERR(_fmt, ...) \
    LOM_RET_SYS_CALL_ERR_WITH_CODE(-1, _fmt, ##__VA_ARGS__)

#define LOM_RET_ON_ERR(_expr) do {  \
    LOM_ERR _lom_err = (_expr);     \
    if (_lom_err) {                 \
        _lom_err->PushTB();         \
        return _lom_err;            \
    }                               \
} while (false)
