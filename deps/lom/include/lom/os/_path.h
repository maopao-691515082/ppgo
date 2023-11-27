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
从一个路径串构建标准化的路径对象
    - 若输入为相对路径，则先通过`getcwd`取当前路径进行拼接形成绝对路径
    - 对绝对路径的处理规则和`NormPath`相同

可直接通过构造函数构建，或用`Make`方法构建，区别在于当传入的路径串为相对路径，在`getcwd`取当前路径失败时，
`Make`返回错误，而构造函数会abort
*/
class Path
{
    GoSlice<::lom::Str> paths_;

    Path(const GoSlice<::lom::Str> &paths) : paths_(paths)
    {
    }

public:

    Path(const Str &path);

    //返回标准化的路径串
    ::lom::Str Str() const;

    /*
    获取当前路径的目录或最后一级文件名
    注意根目录路径`/`比较特殊，其目录和最后一级文件名都是自身
    */
    Path Dir() const
    {
        return paths_.Len() > 0 ? Path(paths_.Slice(0, paths_.Len() - 1)) : *this;
    }
    ::lom::Str Base() const
    {
        return paths_.Len() > 0 ? paths_.At(paths_.Len() - 1) : "/";
    }

    static LOM_ERR Make(const ::lom::Str &path_str, Path &path);
};

}

}
