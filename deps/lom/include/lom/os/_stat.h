#pragma once

#include "../_internal.h"

#include "../err.h"

namespace lom
{

namespace os
{

class FileStat
{
    bool exists_;
    struct stat stat_;

public:

    /*
    判断文件是否存在
    某些方法（如`PermBits`和`Size`只有在文件存在的情况下才有意义）
    */
    bool Exists() const
    {
        return exists_;
    }

    /*
    判断文件类型的快捷方法
        `IsLink()`为true当且仅当`LStat`操作符号链接，且此时其他`IsXXX()`均为false
    */
    bool IsDir() const
    {
        return Exists() && S_ISDIR(stat_.st_mode);
    }
    bool IsFile() const
    {
        return Exists() && S_ISREG(stat_.st_mode);
    }
    bool IsLink() const
    {
        return Exists() && S_ISLNK(stat_.st_mode);
    }

    //获取权限相关信息，和系统定义的`mode_t`的值含义一致（如0755）
    int PermBits() const
    {
        return Exists() ? stat_.st_mode & 0777 : 0;
    }

    //获取文件大小
    ssize_t Size() const
    {
        return Exists() ? static_cast<ssize_t>(stat_.st_size) : -1;
    }

    //系统调用`stat`和`lstat`的封装使用
    static LOM_ERR Stat(const Str &path, FileStat &fst);
    static LOM_ERR LStat(const Str &path, FileStat &fst);
};

}

}
