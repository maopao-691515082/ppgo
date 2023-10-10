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

/*
删除`path`
    - 对于符号链接，删除链接本身
    - 对于文件，正常删除
    - 对于目录，删除整个目录结构（递归进行）
*/
LOM_ERR RemoveTree(const char *path);

//封装`rename`
LOM_ERR Rename(const char *old_path, const char *new_path);

}

}
