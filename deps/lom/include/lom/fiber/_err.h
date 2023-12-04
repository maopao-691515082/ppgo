#pragma once

namespace lom
{

namespace fiber
{

namespace err_code
{

//错误码定义
enum : int
{
    kSysCallFailed  = -1,

    kTimeout    = -2,
    kClosed     = -3,
    kIntr       = -4,
    kInvalid    = -5,
    kCanceled   = -6,
    kConnReset  = -7,
    kBrokenPipe = -8,

    kStdCodeMin = -10000,   //标准错误边界，比这个值小的错误码可由用户定义（若需要的话）
};

}

}

}
