#pragma once

#include "_err.h"
#include "_fd.h"

namespace lom
{

namespace fiber
{

class Conn : public Fd
{
public:

    /*
    读数据，`sz`必须>0
    成功时`rsz`返回结果：
        >0：读到的字节数
        0：文件结束
    */
    ::lom::Err::Ptr Read(char *buf, ssize_t sz, ssize_t &rsz) const;

    /*
    写数据，`sz`必须>=0，`sz`为0时仅做有效性检查，`sz`>0时允许部分成功（写入至少1字节就会成功返回）
    成功时`wsz`返回结果：
        >0：成功写入的字节数
        0：仅当`sz`为0且通过了有效性检查时返回，表示成功
    */
    ::lom::Err::Ptr Write(const char *buf, ssize_t sz, ssize_t &wsz) const;

    //写数据，成功则保证写完，`sz`必须>=0，`sz`为0时仅做有效性检查
    ::lom::Err::Ptr WriteAll(const char *buf, ssize_t sz) const;

    //从一个原始fd创建新的`Conn`对象
    static ::lom::Err::Ptr NewFromRawFd(int fd, Conn &conn);
};

/*
向地址`ip:port`建立TCP连接，IPV4版本
    `ipv4`必须是标准的IPV4格式，不支持hostname或域名
    `port`指定端口
*/
::lom::Err::Ptr ConnectTCP(const char *ipv4, uint16_t port, Conn &conn);

/*
向本地Unix域的流式socket建立连接
`path`指定需要连接的地址，注意是普通C字符串，即`\0`结尾，长度不能超过`sockaddr_un.sun_path`的大小减一
其他输入参数含义和返回行为同`ConnectTCP`
*/
::lom::Err::Ptr ConnectUnixSockStream(const char *path, Conn &conn);

/*
类似`ConnectUnixSockStream`，但是使用Linux的抽象路径机制，输入的`path`不需要带首位的`\0`，接口会自动补上，
因此`path`的长度不能超过`sockaddr_un.sun_path`的大小减一
其他输入参数含义和返回行为同`ConnectUnixSockStream`
*/
::lom::Err::Ptr ConnectUnixSockStreamWithAbstractPath(const Str &path, Conn &conn);

}

}
