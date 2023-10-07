#pragma once

#include "../_internal.h"

#include "../err.h"

namespace lom
{

namespace io
{

/*
调用者可提供这个参数用于指定自定义的缓冲区，buf-io库保证这个`sz`参数在合法区间
返回的缓冲区指针合法性和生命周期由调用者自行保证
    - `Buf{Reader|Writer}`会保存`New`的时候传入的函数对象，
      因此缓冲区的生命周期可以由对应的`Do{Read|Write}Func`函数，或这个函数来绑定
*/
typedef std::function<char * (ssize_t sz)> MakeBufFunc;

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
    typedef std::function<LOM_ERR (char *buf, ssize_t sz, ssize_t &rsz)> DoReadFunc;

    /*
    若缓冲中有数据则立即成功，否则等待到至少有1字节数据进入缓冲，或出错返回
    成功时通过`rsz`返回当前缓冲中的数据长度，为0则表示文件结束
    */
    virtual LOM_ERR Wait(ssize_t &rsz) = 0;

    /*
    读取数据，返回读取到的字节数，不保证读到`sz`大小
    `sz<=0`时返回错误
    `rsz`值含义和`DoReadFunc`注释的内容相同
    */
    virtual LOM_ERR Read(char *buf, ssize_t sz, ssize_t &rsz) = 0;

    /*
    `ReadUntil`、`ReadFull`都可能返回`UnexpectedEOF()`，需要注意本错误可能有两种返回情况：
        - 调用`DoReadFunc`时文件结束，则直接返回
        - `DoReadFunc`函数本身返回的就是本错误，透传返回
    在前者情况中，相关方法的`rsz`参数会被设置为已经读到的数据长度，且`buf`中对应长度的数据合法
    由于有透传`DoReadFunc`返回本错误的可能，如果希望在遇到本错误后使用`buf`和`rsz`返回的部分数据，
    可在调用前先将`rsz`设置为负数值，通过调用后其值是否变化来区分是否第一种情况
    */

    /*
    读取数据，直到读到字节数到达`sz`大小，或者读到EOF，或者读到`end_ch`，或者出错为止
    在指定长度内`end_ch`存在的情况下，成功调用返回的`buf`中的数据必然是以`end_ch`结尾的
    `rsz`返回值表示读到的字节数
        - 若成功读到`end_ch`，则相当于包含`end_ch`在内的内容长度
        - 若成功读满`buf`而未读到`end_ch`，则等于`sz`
        - 若返回`UnexpectedEOF()`，则可能指示已成功读到的数据长度，见上述注释
    */
    virtual LOM_ERR ReadUntil(char end_ch, char *buf, ssize_t sz, ssize_t &rsz) = 0;

    /*
    读取数据，反复读取直到读到字节数到达`sz`大小，或者读到EOF，或者出错为止
    若指定`rsz`，则在返回`UnexpectedEOF()`时可能指示已成功读到的数据长度，见上述注释
    */
    virtual LOM_ERR ReadFull(char *buf, ssize_t sz, ssize_t *rsz = nullptr) = 0;

    static Ptr New(DoReadFunc do_read, ssize_t buf_sz = 0, MakeBufFunc make_buf = nullptr);
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
    typedef std::function<LOM_ERR (const char *buf, ssize_t sz, ssize_t &wsz)> DoWriteFunc;

    //将指定输入数据全部写入`BufWriter`，意即写入缓冲即算成功
    virtual LOM_ERR WriteAll(const char *buf, ssize_t sz) = 0;

    //将缓冲中的数据通过下层写函数全部写出去
    virtual LOM_ERR Flush() = 0;

    static Ptr New(DoWriteFunc do_write, ssize_t buf_sz = 0, MakeBufFunc make_buf = nullptr);
};

}

}
