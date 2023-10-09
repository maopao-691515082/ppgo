#pragma once

#include "../_internal.h"

#include "../err.h"
#include "../go_slice.h"

namespace lom
{

namespace os
{

//获取指定目录下的文件名列表
LOM_ERR ListDir(const char *path, GoSlice<Str> &names);

}

}
