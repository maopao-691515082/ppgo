#pragma once

#include "../_internal.h"

#include "../err.h"
#include "../io/io.h"

namespace lom
{

namespace os
{

//文件对象封装，注意不带缓冲，和libc的`FILE`不一样
class File
{
    int fd_ = -1;

    File(int fd) : fd_(fd)
    {
    }

    File(const File &) = delete;
    File &operator=(const File &) = delete;

public:

    /*
    面向描述字的系统调用封装，以及衍生的一些封装接口
    使用lom风格并规避一些UB或平台行为情况
        - 大小参数使用`ssize_t`并检查负值
        - 返回封装为错误对象
        等等
    */

    /*
    对文件进行加解锁操作
    下层使用`flock`系统调用而不是`fcntl`，其区别以及`flock`的行为请参考系统调用文档
    */
    static LOM_ERR Lock(int fd);
    static LOM_ERR TryLock(int fd, bool &ok);
    static LOM_ERR Unlock(int fd);

    //`seek`封装
    enum SeekWhence
    {
        kSeekSet,
        kSeekCur,
        kSeekEnd,
    };
    static LOM_ERR Seek(int fd, ssize_t off, SeekWhence whence, ssize_t *new_off = nullptr);

    /*
    封装`read`/`write`，成功时通过`rsz`/`wsz`返回已读写的字节数
    和直接使用系统调用的区别：
        - `Read`的参数`sz`必须为正数
        - 遇到`EINTR`错误时重试
    */
    static LOM_ERR Read(int fd, char *buf, ssize_t sz, ssize_t &rsz);
    static LOM_ERR Write(int fd, const char *buf, ssize_t sz, ssize_t &wsz);

    /*
    读取数据，反复读取直到读到字节数到达`sz`大小，或者读到EOF，或者出错为止
    若指定`rsz`，则在返回`UnexpectedEOF()`时指示已成功读到的数据长度
    */
    static LOM_ERR ReadFull(int fd, char *buf, ssize_t sz, ssize_t *rsz = nullptr);

    //写入全部数据
    static LOM_ERR WriteAll(int fd, const char *buf, ssize_t sz);

    //`pread`和`pwrite`的封装版本，参数含义和流程区别参考对应的`read`和`write`封装版本
    static LOM_ERR PRead(int fd, ssize_t off, char *buf, ssize_t sz, ssize_t &rsz);
    static LOM_ERR PWrite(int fd, ssize_t off, const char *buf, ssize_t sz, ssize_t &wsz);
    static LOM_ERR PReadFull(int fd, ssize_t off, char *buf, ssize_t sz, ssize_t *rsz = nullptr);
    static LOM_ERR PWriteAll(int fd, ssize_t off, const char *buf, ssize_t sz);

public:

    typedef std::shared_ptr<File> Ptr;

    ~File()
    {
        ::close(fd_);
    }

    LOM_ERR Lock() const
    {
        return Lock(fd_);
    }
    LOM_ERR TryLock(bool &ok) const
    {
        return TryLock(fd_, ok);
    }
    LOM_ERR Unlock() const
    {
        return Unlock(fd_);
    }

    LOM_ERR Seek(ssize_t off, SeekWhence whence, ssize_t *new_off = nullptr) const
    {
        return Seek(fd_, off, whence, new_off);
    }

    LOM_ERR Read(char *buf, ssize_t sz, ssize_t &rsz) const
    {
        return Read(fd_, buf, sz, rsz);
    }
    LOM_ERR Write(const char *buf, ssize_t sz, ssize_t &wrsz) const
    {
        return Write(fd_, buf, sz, wrsz);
    }
    LOM_ERR ReadFull(char *buf, ssize_t sz, ssize_t *rsz = nullptr) const
    {
        return ReadFull(fd_, buf, sz, rsz);
    }
    LOM_ERR WriteAll(const char *buf, ssize_t sz) const
    {
        return WriteAll(fd_, buf, sz);
    }

    LOM_ERR PRead(ssize_t off, char *buf, ssize_t sz, ssize_t &rsz) const
    {
        return PRead(fd_, off, buf, sz, rsz);
    }
    LOM_ERR PWrite(ssize_t off, const char *buf, ssize_t sz, ssize_t &wsz) const
    {
        return PWrite(fd_, off, buf, sz, wsz);
    }
    LOM_ERR PReadFull(ssize_t off, char *buf, ssize_t sz, ssize_t *rsz = nullptr) const
    {
        return PReadFull(fd_, off, buf, sz, rsz);
    }
    LOM_ERR PWriteAll(ssize_t off, const char *buf, ssize_t sz) const
    {
        return PWriteAll(fd_, off, buf, sz);
    }

    /*
    打开文件，可通过`mode`指定打开方式
    `mode`的含义详细说明：
        - 必须以下面几个开头：
            - `r`   打开已存在的文件，只读
            - `r+`  打开已存在的文件，可读写
            - `w`   如文件不存在则创建，否则将内容清空，只写
            - `w+`  如文件不存在则创建，否则将内容清空，可读写
            - `a`   如文件不存在则创建，否则打开，追加写
            - `a+`  如文件不存在则创建，否则打开，追加写并可读
        - 和系统的`open`不同，本接口会默认加上`O_CLOEXEC`选项
        - 其后可跟其他可选模式参数：
            - `E`   关闭`O_CLOEXEC`选项，即强制指定在成功调用`exec`族函数时继承描述字
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
