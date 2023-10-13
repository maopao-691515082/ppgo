#pragma once

#include "_internal.h"

#include "limit.h"

namespace lom
{

static const ssize_t kStrLenMax = kSSizeSoftMax;

static const char *const kSpaceBytes = "\t\r\n\f\v\x20";

template <typename T>
class GoSlice;

class Str;

/*
`StrSlice`用于引用一段连续的char数据
由于只是简单引用，因此使用者自行保证有效性，即作用域或生存期小于数据源
索引存取的输入合法性由调用者保证
*/
class StrSlice final
{
    const char *p_;
    ssize_t len_;

    ssize_t CheckIdx(ssize_t idx, bool allow_end) const
    {
        auto len = Len();
        Assert(0 <= idx && idx <= (allow_end ? len : len - 1));
        return len;
    }

    StrSlice(std::nullptr_t) = delete;
    StrSlice &operator=(std::nullptr_t) = delete;

public:

    //通用构造器，调用者自行保证输入合法性
    StrSlice(const char *p, ssize_t len) : p_(p), len_(len)
    {
        Assert(0 <= len && len <= kStrLenMax);
    }

    //空切片
    StrSlice() : StrSlice("", 0)
    {
    }

    //从各种字符串构建
    StrSlice(const char *s) : StrSlice(s, static_cast<ssize_t>(strlen(s)))
    {
    }
    StrSlice(const std::string &s) : StrSlice(s.c_str(), static_cast<ssize_t>(s.size()))
    {
    }
    StrSlice(const Str &s);

    const char *Data() const
    {
        return p_;
    }
    ssize_t Len() const
    {
        return len_;
    }

    char Get(ssize_t idx) const
    {
        CheckIdx(idx, false);
        return Data()[idx];
    }

    //字典序比较
    int Cmp(StrSlice s) const
    {
        int ret = memcmp(Data(), s.Data(), std::min(Len(), s.Len()));
        return ret != 0 ? ret : (
            Len() == s.Len() ? 0 : (
                Len() > s.Len() ? 1 : -1
            )
        );
    }

    //运算符也统一实现一下，某些标准库容器需要
    bool operator<  (StrSlice s) const { return Cmp(s) <    0; }
    bool operator<= (StrSlice s) const { return Cmp(s) <=   0; }
    bool operator>  (StrSlice s) const { return Cmp(s) >    0; }
    bool operator>= (StrSlice s) const { return Cmp(s) >=   0; }
    bool operator== (StrSlice s) const { return Cmp(s) ==   0; }
    bool operator!= (StrSlice s) const { return Cmp(s) !=   0; }

    //忽略大小写比较是否相等
    bool CaseEq(StrSlice s) const;

    //指定起始索引和长度返回新的串切片，不指定长度则表示后半段，返回的和当前串引用同一个数据源
    StrSlice Slice(ssize_t start, ssize_t len) const
    {
        auto this_len = CheckIdx(start, true);
        auto cap = this_len - start;
        Assert(0 <= len && len <= cap);
        return StrSlice(Data() + start, len);
    }
    StrSlice Slice(ssize_t start) const
    {
        auto this_len = CheckIdx(start, true);
        return StrSlice(Data() + start, this_len - start);
    }

    //正向或反向查找字节，返回索引，不存在返回-1
    ssize_t IndexByte(unsigned char b) const
    {
        auto p = reinterpret_cast<const char *>(memchr(Data(), b, Len()));
        return p == nullptr ? -1 : p - Data();
    }
    ssize_t IndexChar(char c) const
    {
        return IndexByte(static_cast<unsigned char>(c));
    }
    ssize_t RIndexByte(unsigned char b) const
    {
        auto data = Data();
        for (ssize_t i = Len() - 1; i >= 0; -- i)
        {
            if (static_cast<unsigned char>(data[i]) == b)
            {
                return i;
            }
        }
        return -1;
    }
    ssize_t RIndexChar(char c) const
    {
        return RIndexByte(static_cast<unsigned char>(c));
    }
    //判断是否包含字节的快捷方式
    bool ContainsByte(unsigned char b) const
    {
        return IndexByte(b) >= 0;
    }
    bool ContainsChar(char c) const
    {
        return IndexChar(c) >= 0;
    }

    //正向或反向查找子串，返回索引，不存在返回-1
    ssize_t Index(StrSlice s) const
    {
        auto p = reinterpret_cast<const char *>(memmem(Data(), Len(), s.Data(), s.Len()));
        return p == nullptr ? -1 : p - Data();
    }
    ssize_t RIndex(StrSlice s) const
    {
        auto data = Data(), s_data = s.Data();
        auto len = Len(), s_len = s.Len();
        if (len >= s_len)
        {
            for (auto i = len - s_len; i >= 0; -- i)
            {
                if (memcmp(data + i, s_data, s_len) == 0)
                {
                    return i;
                }
            }
        }
        return -1;
    }
    //判断是否包含子串的快捷方式
    bool Contains(StrSlice s) const
    {
        return Index(s) >= 0;
    }

    //判断是否含有前后缀
    bool HasPrefix(StrSlice s) const
    {
        auto s_len = s.Len();
        return Len() >= s_len && memcmp(Data(), s.Data(), s_len) == 0;
    }
    bool HasSuffix(StrSlice s) const
    {
        auto this_len = Len(), s_len = s.Len();
        return this_len >= s_len && memcmp(Data() + (this_len - s_len), s.Data(), s_len) == 0;
    }

    /*
    返回移除左、右或两端的所有存在于指定字符集合中的字符后的切片，指定字符集合默认为空白字符集，
    返回的和当前串引用同一个数据源
    */
    StrSlice LTrim(StrSlice chs = kSpaceBytes) const
    {
        ssize_t this_len = Len(), i = 0;
        while (i < this_len && chs.ContainsChar(Data()[i]))
        {
            ++ i;
        }
        return Slice(i);
    }
    StrSlice RTrim(StrSlice chs = kSpaceBytes) const
    {
        auto this_len = Len(), i = this_len - 1;
        while (i >= 0 && chs.ContainsChar(Data()[i]))
        {
            -- i;
        }
        return Slice(0, i + 1);
    }
    StrSlice Trim(StrSlice chs = kSpaceBytes) const
    {
        return LTrim(chs).RTrim(chs);
    }

    /*
    作为字符串来解析整数或浮点数，解析整数时可指定进制，进制为0表示自动按照前缀进行，返回是否成功
    注意和标准库的解析方式不同的是，这些接口不允许前导空白符，必须整个串是一个严格完整的数字表示才成功，
    在类似需求下可以先用`Trim`方法去除空白等字符
    */
    bool ParseInt64(int64_t &v, int base = 0) const;
    bool ParseUInt64(uint64_t &v, int base = 0) const;
    bool ParseFloat(float &v) const;
    bool ParseDouble(double &v) const;
    bool ParseLongDouble(long double &v) const;

    /*
    返回串的可视化表示，规则：
        - 表示的两端用单引号括起来
        - 常见转义字符用其转义形式表示：`\a \b \f \n \r \t \v`
        - 反斜杠和单引号也用转义形式表示：`\\ \'`
        - 其余字符中，编号`32~126`范围内的字符原样表示
        - 剩余字符用两位16进制转义的形式表示：`\xHH`
    */
    Str Repr() const;

    /*
    根据`sep`参数分割串，返回被分割后的各部分构成的`GoSlice`，规则：
        - 若`sep`为空串，则根据连续空白符分割，且忽略两端空白符
            - 例如：`"  \ta   \v b   \n\rc"`被分割为`["a", "b", "c"]`
        - 若`sep`不为空串，则严格按照`sep`分割，若出现K个`sep`，则严格分割为`K+1`部分
            - 例如：`"|a|bc|  d||e"`按`"|"`分割，结果是`["", "a", "bc", "  d", "", "e"]`
        - 返回的和当前串引用同一个数据源
    */
    GoSlice<StrSlice> Split(StrSlice sep) const;

    //将所包含的所有字母转为大写或小写，返回转换后的Str对象
    Str Upper() const;
    Str Lower() const;

    /*
    `Hex`将串转为16进制的形式，每个字符用两位`HH`表示，例如`"1+2"`转为`"312B32"`
    `Unhex`执行反向操作，返回是否成功，若输入的不是合法的形式，则返回失败
    */
    Str Hex(bool upper_case = true) const;
    bool Unhex(Str &s) const;

    //以当前串为分隔符，链接输入的GoSlice中的所有串，返回结果
    Str Join(const GoSlice<StrSlice> &gs) const;
    Str Join(const GoSlice<Str> &gs) const;

    /*
    以当前串为分隔符，用输入的迭代`next`函数提供的序列进行连接，返回结果
    `next`返回true表示成功获取，false表示结束
    */
    Str Join(std::function<bool (StrSlice &)> next, ssize_t size_hint = 0) const;

    /*
    在当前串中查找子串`a`，并返回等同于将其替换为指定串的结果的新Str
    第一种形式是通过f返回需要替换成的指定内容，每次替换都会调用一次`f`，第二种形式则是直接指定串`b`作为内容
    `max_count`可限定最大替换次数
    每一次替换完成后，是从剩余的串开始查找下一个串`a`，
    例如`StrSlice("xxx").Replace("x", "xx")`会返回`"xxxxxx"`，而不会永久循环
    */
    Str Replace(StrSlice a, std::function<StrSlice ()> f, ssize_t max_count = kStrLenMax) const;
    Str Replace(StrSlice a, StrSlice b, ssize_t max_count = kStrLenMax) const;

    //字符串连接，返回连接后的结果
    Str Concat(StrSlice s) const;
};

/*
自实现的字符串类，和Java、Go等语言的字符串类相同，可视为引用不可变字符串的引用类型
`Str`的大部分方法算法都是将自身转为`StrSlice`后，再调用其对应方法，以下这种情况不再单独注释说明
和大部分字符串实现一样，`Str`会在有效数据末尾额外申请一个字节并存放`\0`，从而在某些方法下兼容C风格
*/
class Str final
{
    //以这个结构体的指针指向申请的长串内存，然后用`shared_ptr`管理
    struct LongStr
    {
        char s_[1];
    };

    /*
    `Str`结构和算法说明：
        - 由于`limit.h`已经限定了指针是8字节，在字节对齐的情况下，
          STL的`shared_ptr`或其他智能指针的实现大概率是16字节，或其他8的倍数，
          所以这里就通过额外8字节的头部+`shared_ptr`大小来安排`Str`对象，若`lsp_`前出现padding也没关系
        - `ss`（short-string），`ss_len_`字段表示短串长度，若为负数则表示当前`Str`是一个长串
            - 短串存储是将`Str`结构中`ss_len_`之后的空间，即`ss_start_`开始的空间，看做一段内存直接存储字符串
            - 长串存储是用`ls_len_high_`和`ls_len_low_`分别存储长度的高16位和低32位，然后由`lsp_`管理具体长串
        - 由于含有末尾`\0`，再扣去`ss_len_`，则支持的短串长度为[0, sizeof(Str) - 2]

    我们可以假设下述字段如所期望的布局方式存储，
    并在下面的`IsLongStr`方法中做个静态断言检查以确保这个设计能正常运作
    */
    int8_t ss_len_;
    char ss_start_;
    uint16_t ls_len_high_;
    uint32_t ls_len_low_;
    union
    {
        std::shared_ptr<LongStr> lsp_;
        char ud_[sizeof(lsp_)]; //plain union data
    };

    bool IsLongStr() const
    {
        //对上述假设做一个静态断言检查
        static_assert(
            sizeof(LongStr) == 1 && sizeof(lsp_) % 8 == 0 &&
            offsetof(Str, ss_len_) == 0 && offsetof(Str, ss_start_) == 1 &&
            offsetof(Str, lsp_) == offsetof(Str, ud_) && offsetof(Str, lsp_) + sizeof(lsp_) == sizeof(Str),
            "unsupportted string fields arrangement");

        return ss_len_ < 0;
    }

    void Destruct()
    {
        if (IsLongStr())
        {
            lsp_.~shared_ptr();
        }
    }

    void Assign(const Str &s)
    {
        if (s.IsLongStr())
        {
            ss_len_ = s.ss_len_;
            ss_start_ = s.ss_start_;
            ls_len_high_ = s.ls_len_high_;
            ls_len_low_ = s.ls_len_low_;
            new (&lsp_) std::shared_ptr<LongStr>(s.lsp_);
        }
        else
        {
            memcpy(&ss_len_, &s.ss_len_, offsetof(Str, ud_));
            memcpy(ud_, s.ud_, sizeof(ud_));
        }
    }

    void MoveFrom(Str &&s)
    {
        if (s.IsLongStr())
        {
            ss_len_ = s.ss_len_;
            ss_start_ = s.ss_start_;
            ls_len_high_ = s.ls_len_high_;
            ls_len_low_ = s.ls_len_low_;
            new (&lsp_) std::shared_ptr<LongStr>(std::move(s.lsp_));
        }
        else
        {
            memcpy(&ss_len_, &s.ss_len_, offsetof(Str, ud_));
            memcpy(ud_, s.ud_, sizeof(ud_));
        }
        s.ss_len_ = 0;
        s.ss_start_ = '\0';
    }

    Str(std::nullptr_t) = delete;
    Str &operator=(std::nullptr_t) = delete;

public:

    //从其他类型的数据转`Str`都通过`StrSlice`转统一流程
    Str(StrSlice s);
    Str() : Str(StrSlice())
    {
    }
    Str(const char *s) : Str(StrSlice(s))
    {
    }
    Str(const char *p, ssize_t len) : Str(StrSlice(p, len))
    {
    }
    Str(const std::string &s) : Str(StrSlice(s))
    {
    }

    Str(const Str &s)
    {
        Assign(s);
    }

    //移动构造会将`s`的内容迁移到当前对象，并将`s`置空，移动赋值操作也是一样的机制
    Str(Str &&s)
    {
        MoveFrom(std::move(s));
    }

    ~Str()
    {
        Destruct();
    }

    Str &operator=(StrSlice s)
    {
        Str tmp(s);
        Destruct();
        MoveFrom(std::move(tmp));
        return *this;
    }

    Str &operator=(const char *s)
    {
        return operator=(StrSlice(s));
    }

    Str &operator=(const std::string &s)
    {
        return operator=(StrSlice(s));
    }

    Str &operator=(const Str &s)
    {
        if (this != &s)
        {
            Destruct();
            Assign(s);
        }
        return *this;
    }

    Str &operator=(Str &&s)
    {
        if (this != &s)
        {
            Destruct();
            MoveFrom(std::move(s));
        }
        return *this;
    }

    const char *Data() const
    {
        return IsLongStr() ? lsp_->s_ : &ss_start_;
    }
    ssize_t Len() const
    {
        return IsLongStr() ? (static_cast<ssize_t>(ls_len_high_) << 32) + ls_len_low_ : ss_len_;
    }
    //类似STL的`string`的`c_str`方法，`Str`对象会保证末尾有额外的`\0`，所以直接按C风格字符串访问是没问题的
    const char *CStr() const
    {
        return Data();
    }

    StrSlice Slice() const
    {
        return StrSlice(*this);
    }

    char Get(ssize_t idx) const
    {
        return Slice().Get(idx);
    }

    int Cmp(StrSlice s) const
    {
        return Slice().Cmp(s);
    }

    bool operator<  (StrSlice s) const { return Cmp(s) <    0; }
    bool operator<= (StrSlice s) const { return Cmp(s) <=   0; }
    bool operator>  (StrSlice s) const { return Cmp(s) >    0; }
    bool operator>= (StrSlice s) const { return Cmp(s) >=   0; }
    bool operator== (StrSlice s) const { return Cmp(s) ==   0; }
    bool operator!= (StrSlice s) const { return Cmp(s) !=   0; }

    bool CaseEq(StrSlice s) const
    {
        return Slice().CaseEq(s);
    }

    ssize_t IndexByte(unsigned char b) const
    {
        return Slice().IndexByte(b);
    }
    ssize_t IndexChar(char c) const
    {
        return Slice().IndexChar(c);
    }
    ssize_t RIndexByte(unsigned char b) const
    {
        return Slice().RIndexByte(b);
    }
    ssize_t RIndexChar(char c) const
    {
        return Slice().RIndexChar(c);
    }
    bool ContainsByte(unsigned char b) const
    {
        return Slice().ContainsByte(b);
    }
    bool ContainsChar(char c) const
    {
        return Slice().ContainsChar(c);
    }

    ssize_t Index(StrSlice s) const
    {
        return Slice().Index(s);
    }
    ssize_t RIndex(StrSlice s) const
    {
        return Slice().RIndex(s);
    }
    bool Contains(StrSlice s) const
    {
        return Slice().Contains(s);
    }

    bool HasPrefix(StrSlice s) const
    {
        return Slice().HasPrefix(s);
    }
    bool HasSuffix(StrSlice s) const
    {
        return Slice().HasSuffix(s);
    }

    Str LTrim(StrSlice chs = kSpaceBytes) const
    {
        return Slice().LTrim(chs);
    }
    Str RTrim(StrSlice chs = kSpaceBytes) const
    {
        return Slice().RTrim(chs);
    }
    Str Trim(StrSlice chs = kSpaceBytes) const
    {
        return Slice().Trim(chs);
    }

    bool ParseInt64(int64_t &v, int base = 0) const;
    bool ParseUInt64(uint64_t &v, int base = 0) const;
    bool ParseFloat(float &v) const;
    bool ParseDouble(double &v) const;
    bool ParseLongDouble(long double &v) const;

    Str Repr() const
    {
        return Slice().Repr();
    }

    //算法同`StrSlice`的`Split`，但返回的`GoSlice`的元素是`Str`对象类型
    GoSlice<Str> Split(StrSlice sep) const;

    Str SubStr(ssize_t start, ssize_t len) const
    {
        return Slice().Slice(start, len);
    }
    Str SubStr(ssize_t start) const
    {
        return Slice().Slice(start);
    }

    /*
    `Buf`对象可看做是一段可写、可追加的字节区，一般用于构建字符串，不可拷贝构建或赋值
    会保证逻辑数据后有一个额外的`\0`字节，但是调用者不应该去修改它，否则行为未定义
    */
    class Buf
    {
        char *p_;
        ssize_t len_;
        ssize_t cap_;

        friend class Str;

        Buf(const Buf &) = delete;
        Buf &operator=(const Buf &) = delete;

        void Construct(ssize_t len, ssize_t cap)
        {
            Assert(0 <= len && len <= cap && cap <= kStrLenMax);
            p_ = reinterpret_cast<char *>(malloc(cap + 1));
            Assert(p_ != nullptr);
            p_[len] = '\0';
            len_ = len;
            cap_ = cap;
        }

        void Construct()
        {
            Construct(0, 16);
        }

    public:

        //可构建空`Buf`或指定`len`和`cap`构建，`len`和`cap`的合法性由调用者保证
        Buf()
        {
            Construct();
        }
        Buf(ssize_t len, ssize_t cap)
        {
            Construct(len, cap);
        }
        Buf(ssize_t len) : Buf(len, len)
        {
        }

        ~Buf()
        {
            free(p_);
        }

        /*
        返回当前`Buf`的数据指针和长度，注意由于`Buf`内部缓冲区是可扩缩的，这个指针和长度有可能失效，
        需要调用者自行保证合法使用，大部分时候只需要安全使用下面的`Write`和`Append`方法即可
        */
        char *Data() const
        {
            return p_;
        }
        ssize_t Len() const
        {
            return len_;
        }
        ssize_t Cap() const
        {
            return cap_;
        }

        //将`Buf`扩展到指定长度，若当前长度已>=`len`则不做变化
        void FitLen(ssize_t len);

        //指定偏移写数据内容，若当前长度不足则会自动扩展到需要的长度
        void Write(ssize_t offset, const char *p, ssize_t len)
        {
            Assert(0 <= offset && offset <= len_ && 0 <= len && len <= kStrLenMax - offset);
            FitLen(offset + len);
            memcpy(p_ + offset, p, len);
        }
        void Write(ssize_t offset, StrSlice s)
        {
            Write(offset, s.Data(), s.Len());
        }

        //追加数据
        void Append(const char *p, ssize_t len)
        {
            Write(len_, p, len);
        }
        void Append(StrSlice s)
        {
            Write(len_, s);
        }
    };

private:

    void MoveFrom(Buf &&buf);

public:

    /*
    从`Buf`对象移动构建或移动赋值`Str`，`buf`本身会被初始化为空
    对于长串，是将`buf`维护的下层数据直接移动给`Str`对象，所以性能比较好，
    但是可能会因为`buf`的构建过程浪费一定空间（即`buf`的`cap_`比`len_`要大）
    */
    Str(Buf &&buf)
    {
        MoveFrom(std::move(buf));
    }
    Str &operator=(Buf &&buf)
    {
        Destruct();
        MoveFrom(std::move(buf));
        return *this;
    }

    //将整数转为字符串（十进制），这里的实现比STL稍快一些（有的STL实现用`snprintf`）
    static Str FromInt64(int64_t n);
    static Str FromUInt64(uint64_t n);

    Str Upper() const
    {
        return Slice().Upper();
    }
    Str Lower() const
    {
        return Slice().Lower();
    }

    Str Hex(bool upper_case = true) const
    {
        return Slice().Hex(upper_case);
    }
    bool Unhex(Str &s) const
    {
        return Slice().Unhex(s);
    }

    Str Join(const GoSlice<StrSlice> &gs) const;
    Str Join(const GoSlice<Str> &gs) const;

    Str Replace(StrSlice a, std::function<StrSlice ()> f, ssize_t max_count = kStrLenMax) const
    {
        return Slice().Replace(a, f, max_count);
    }
    Str Replace(StrSlice a, StrSlice b, ssize_t max_count = kStrLenMax) const
    {
        return Slice().Replace(a, b, max_count);
    }

    Str Concat(StrSlice s) const
    {
        return Slice().Concat(s);
    }
};

/*
类似标准库的`sprintf`，但不是打印到给定buf，而是打印成一个`Str`对象
输入参数语法和`printf`族的规定一致
*/
[[gnu::format(gnu_printf, 1, 2)]]
Str Sprintf(const char *fmt, ...);

}
