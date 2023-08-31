#pragma once

#include "../_internal.h"

#include "../mem.h"

namespace lom
{

namespace io
{

/*
带缓冲的读封装，通过传入一个下层读函数来构建
可指定缓冲大小，但会被调整到一个内部范围，指定<=0表示使用默认值
*/
class BufReader
{
public:

    typedef std::shared_ptr<BufReader> Ptr;

    virtual ~BufReader() = default;

    /*
    下层的读函数类型
    由于本对象的读取方法在`sz<=0`时返回无效参数错误，因此调用下层读函数时必然是`sz>0`
    成功时需要用`rsz`返回结果：
        >0：读到对应长度的数据，若大于`sz`则行为未定义
        0：文件结束
        <0：行为未定义
    失败时向上返回错误对象
    */
    typedef std::function<::lom::Err::Ptr (char *buf, ssize_t sz, ssize_t &rsz)> DoReadFunc;

    /*
    读取数据，返回读取到的字节数，不保证读到`sz`大小
    `sz<=0`时返回错误
    `rsz`值含义和`DoReadFunc`注释的内容相同
    */
    virtual ::lom::Err::Ptr Read(char *buf, ssize_t sz, ssize_t &rsz) = 0;

    /*
    读取数据，直到读到字节数到达`sz`大小，或者读到EOF，或者读到`end_ch`，或者出错为止
    在`end_ch`之前读到EOF不算出错，若已经读到一些数据，会通过`rsz`返回读到的字符数（注：可能是0）
    其他情况下`rsz`值含义同`Read`
    在指定长度内`end_ch`存在的情况下，成功调用返回的`buf`中的数据必然是以`end_ch`结尾的，
    若读取到的数据不以`end_ch`结尾并且`rsz`小于`sz`，则表示到了EOF
    */
    virtual ::lom::Err::Ptr ReadUntil(char end_ch, char *buf, ssize_t sz, ssize_t &rsz) = 0;

    /*
    读取数据，反复读取直到读到字节数到达`sz`大小，或者读到EOF，或者出错为止
    在读够`sz`大小前读到EOF不算出错，会通过`rsz`返回读到的数据长度（注：可能是0），因此可通过这点判断是否EOF
    其他情况下`rsz`值含义同`Read`
    */
    virtual ::lom::Err::Ptr ReadFull(char *buf, ssize_t sz, ssize_t &rsz) = 0;

    static Ptr New(DoReadFunc do_read, ssize_t buf_sz = 0);
};

/*
带缓冲的写封装，通过传入一个下层写函数来构建
可指定缓冲大小，但会被调整到一个内部范围，指定<=0表示使用默认值
*/
class BufWriter
{
public:

    typedef std::shared_ptr<BufWriter> Ptr;

    virtual ~BufWriter() = default;

    /*
    下层的写函数类型
    由于本对象的写接口在`sz<0`时返回无效参数错误，在`sz==0`时立即返回成功，
    因此调用下层写函数时`sz`必然>0
    下层写函数不需要保证将数据完全发送，能发送一部分就行
    成功时需要用`wsz`返回写成功的数据长度，调用者需保证此时`0<wsz<=sz`，否则行为未定义
    */
    typedef std::function<::lom::Err::Ptr (const char *buf, ssize_t sz, ssize_t &wsz)> DoWriteFunc;

    //将指定输入数据全部写入`BufWriter`，意即写入缓冲即算成功
    virtual ::lom::Err::Ptr WriteAll(const char *buf, ssize_t sz) = 0;

    //将缓冲中的数据通过下层写函数全部写出去
    virtual ::lom::Err::Ptr Flush() = 0;

    static Ptr New(DoWriteFunc do_write, ssize_t buf_sz = 0);
};

}

}
