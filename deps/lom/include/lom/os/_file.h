#pragma once

#include "../_internal.h"

#include "../err.h"

namespace lom
{

namespace os
{

//文件描述字的对象封装，注意不带缓冲，和libc的`FILE`不一样
class File
{
    int fd_ = -1;

    File(int fd) : fd_(fd)
    {
    }

    File(const File &) = delete;
    File &operator=(const File &) = delete;

public:

    typedef std::shared_ptr<File> Ptr;

    ~File()
    {
        ::close(fd_);
    }

    /*
    对文件进行加解锁操作
    下层使用`flock`系统调用而不是`fcntl`，其区别以及`flock`的行为请参考系统调用文档
    */
    LOM_ERR Lock() const;
    LOM_ERR TryLock(bool &ok) const;
    LOM_ERR Unlock() const;

    /*
    打开文件，可通过`mode`指定打开方式
    `mode`的含义基本等同于libc的`fopen`的模式参数（含GNU扩展），详细说明：
        - 必须以下面几个开头：
            - `r`   打开已存在的文件，只读
            - `r+`  打开已存在的文件，可读写
            - `w`   如文件不存在则创建，否则将内容清空，只写
            - `w+`  如文件不存在则创建，否则将内容清空，可读写
            - `a`   如文件不存在则创建，否则打开，追加写
            - `a+`  如文件不存在则创建，否则打开，追加写并可读
                - 注：`a+`模式打开后，写文件总是追加的，但写操作不影响读偏移，初始读偏移在文件开头
        - 其后可跟其他可选模式参数：
            - `e`   打开文件时附加`O_CLOEXEC`选项，即在成功调用`exec`族函数时自动关闭
            - `x`   和`w w+ a a+`配合使用，附加`O_EXCL`选项，即确保创建新文件，如果文件存在则返回错误
                - 若`x`和`r r+`配合使用，则被忽略
                - 注意使用`x`参数时不会解析符号链接，也就是说，目标文件是一个符号链接时，就算它是悬空链接，
                  也会报错
            - `,`   可选模式参数结束，后面是可能的扩展描述
                - 例如`r+e,coding=utf8`之类的样式，虽然现在还不支持
            - 在`,`之前，可选模式参数字符重复出现，或出现其他字符不会导致报错
                - 不识别的参数字符会被忽略掉
                - 如果存在参数之间含义冲突的情况，以最后一个为准
    `perm_bits`指定了创建新文件时的权限，只有存在创建文件的行为时才有意义，否则被忽略
        - 当然，实际创建的文件权限会受到umask影响
    */
    static LOM_ERR Open(const char *path, Ptr &fp, const char *mode = "r", int perm_bits = 0644);
};

}

}
