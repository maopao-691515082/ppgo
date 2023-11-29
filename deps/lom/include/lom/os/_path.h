#pragma once

#include "../_internal.h"

#include "../err.h"
#include "../go_slice.h"

namespace lom
{

namespace os
{

/*
将一个路径串标准化
    - 会处理掉`.`、`..`等特殊路径，连续分隔`//`，末尾`/`等
        - 例如：`/A/./B`、`/A/foo/../B`、`/A/B/`、`////A//B//////`等都会标准化为`/A/B`
    - 输入路径串如果以`/`开头，则认为是绝对路径，否则视为相对路径，返回结果会维持路径的绝对或相对性
        - 若为相对路径
            - 若输入为空串或结果为当前路径，则返回`.`
            - 若结果是以若干个`..`目录开头，则将它们保留
                - 例如：`A/foo/../B`会标准化为`A/B`，`A/../../../B`会标准化为`../../B`
        - 若为绝对路径，按惯例，根目录的所在目录是自身（例如：`/..`不是错误，而视为`/`）
    - 不会扩展`~`和`~<USER>`开头的路径，将其视为普通路径名
*/
::lom::Str NormPath(const Str &path);

/*
将一个路径转为绝对路径
    - 若`path`本身为绝对路径，则过程等同于`NormPath`
    - 若`path`为相对路径，则获取当前工作路径并和其拼接，之后计算`NormPath`
*/
LOM_ERR AbsPath(const Str &path, Str &abs_path);

}

}
