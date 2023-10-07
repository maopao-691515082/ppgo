#pragma once

#include "../_internal.h"

#include "../err.h"

namespace lom
{

namespace io
{

//一个全局唯一错误，用于指示一个完整数据读取类型的过程（如`ReadFull`）因EOF而未完成
LOM_ERR UnexpectedEOF();

}

}
