#pragma once

#include "../_internal.h"

#include "../err.h"

namespace lom
{

namespace io
{

/*
面向文件描述字的各种下层IO操作的封装，以及衍生的封装接口
使用lom风格并规避一些UB或平台行为情况
    - 大小参数使用`ssize_t`并检查负值
    - 返回封装为错误对象
    等等
*/
namespace fds
{

/*
封装`read`/`write`，成功时通过`rsz`/`wsz`返回已读写的字节数
和直接使用系统调用的区别：
    - `Read`的参数`sz`必须为正数
    - 遇到`EINTR`错误时重试
*/
LOM_ERR Read(int fd, char *buf, ssize_t sz, ssize_t &rsz);
LOM_ERR Write(int fd, const char *buf, ssize_t sz, ssize_t &wsz);

/*
读取数据，反复读取直到读到字节数到达`sz`大小，或者读到EOF，或者出错为止
若指定`rsz`，则在返回`UnexpectedEOF()`时指示已成功读到的数据长度
*/
LOM_ERR ReadFull(int fd, char *buf, ssize_t sz, ssize_t *rsz = nullptr);

//写入全部数据
LOM_ERR WriteAll(int fd, const char *buf, ssize_t sz);

}

}

}
