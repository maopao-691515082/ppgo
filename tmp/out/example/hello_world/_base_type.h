#pragma once

namespace ppgo
{

typedef bool        tp_bool;
typedef uint8_t     tp_byte;
typedef int64_t     tp_int;
typedef uint64_t    tp_uint;
typedef double      tp_float;

typedef int8_t      tp_i8;
typedef uint8_t     tp_u8;
typedef int16_t     tp_i16;
typedef uint16_t    tp_u16;
typedef int32_t     tp_i32;
typedef uint32_t    tp_u32;
typedef int64_t     tp_i64;
typedef uint64_t    tp_u64;
typedef float       tp_float32;
typedef double      tp_float64;

class tp_string final
{
    struct LongStr
    {
        std::atomic<int64_t> rc_;
        const char *p_;

        LongStr(const char *p) : rc_(1), p_(p)
        {
        }

        ~LongStr()
        {
            delete[] p_;
        }
    };

    int8_t ss_len_;
    char ss_start_;
    uint16_t ls_len_high_;
    uint32_t ls_len_low_;
    LongStr *lsp_;

    bool IsLongStr() const
    {
        static_assert(
            sizeof(tp_string) == 16 && offsetof(tp_string, ss_len_) == 0 &&
            offsetof(tp_string, ss_start_) == 1,
            "unsupportted string fields arrangement");

        return ss_len_ < 0;
    }

    void Destruct() const
    {
        if (IsLongStr() && lsp_->rc_.fetch_add(-1) == 1)
        {
            delete lsp_;
        }
    }

    void Assign(const tp_string &s)
    {
        if (s.IsLongStr())
        {
            s.lsp_->rc_.fetch_add(1);
        }
        memcpy(this, &s, sizeof(tp_string));
    }

    void AssignLongStr(const char *p, ssize_t len);

public:

    tp_string(const char *p, ssize_t len);

    tp_string() : tp_string("", 0)
    {
    }

    tp_string(const tp_string &s)
    {
        Assign(s);
    }

    ~tp_string()
    {
        Destruct();
    }

    tp_string &operator=(const tp_string &s)
    {
        if (this != &s)
        {
            Destruct();
            Assign(s);
        }
        return *this;
    }

    const char *Data() const
    {
        return IsLongStr() ? lsp_->p_ : &ss_start_;
    }
    ssize_t Len() const
    {
        return IsLongStr() ? ((ssize_t)ls_len_high_ << 32) + (ssize_t)ls_len_low_ : ss_len_;
    }

    static tp_string Sprintf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
};

class Exc;

class Any : public RCObj
{
public:

    typedef RCPtr<Any> Ptr;

    virtual const char *TypeName() const = 0;

    virtual RCPtr<Exc> func_str(std::tuple<tp_string> &ret)
    {
        std::get<0>(ret) = tp_string::Sprintf(
            "<object of type '%s' at 0x%llX>", TypeName(), (unsigned long long)this);
        return nullptr;
    }
};

namespace base_type_boxing
{

class StrObj : public virtual Any
{
    tp_string s_;

public:

    virtual const char *TypeName() const override
    {
        return "string";
    }

    virtual RCPtr<Exc> func_str(std::tuple<tp_string> &ret) override
    {
        std::get<0>(ret) = s_;
        return nullptr;
    }

    static Any::Ptr New(tp_string s)
    {
        auto so = new StrObj;
        so->s_ = s;
        return (Any *)so;
    }
};

}

}
