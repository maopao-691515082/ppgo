#pragma once

#include "../_internal.h"

#include "_path.h"
#include "_stat.h"

namespace lom
{

namespace os
{

//根据`path`递归建立整个目录结构，若`path`已经是一个目录也成功返回
::lom::Err::Ptr MakeDirs(const Path &path, int perm_bits = 0777);

}

}
