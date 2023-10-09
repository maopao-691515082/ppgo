#pragma once

#include "../_internal.h"

#include "_dir.h"
#include "_file.h"
#include "_path.h"
#include "_stat.h"

namespace lom
{

namespace os
{

//根据`path`递归建立整个目录结构，若`path`已经是一个目录也成功返回
LOM_ERR MakeDirs(const Path &path, int perm_bits = 0777);

}

}
