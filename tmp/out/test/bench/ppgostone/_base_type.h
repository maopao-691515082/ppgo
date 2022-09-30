#pragma once

namespace ppgo
{

typedef bool        tp_bool;
typedef uint8_t     tp_byte;
typedef int64_t     tp_int;
typedef uint64_t    tp_uint;
typedef double      tp_float;

typedef int8_t      tp_i8;
struct tp_u8 { uint8_t v_; };
typedef int16_t     tp_i16;
typedef uint16_t    tp_u16;
typedef int32_t     tp_i32;
typedef uint32_t    tp_u32;
struct tp_i64 { int64_t v_; };
struct tp_u64 { uint64_t v_; };
typedef float       tp_float32;
struct tp_float64 { double v_; };

class tp_string final
{
    struct LongStr
    {
        RC rc_;
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

    tp_string(const char *s) : tp_string(s, strlen(s))
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

    int Cmp(const tp_string &s) const
    {
        auto data = Data(), s_data = s.Data();
        auto len = Len(), s_len = s.Len();
        int ret = memcmp(data, s_data, std::min(len, s_len));
        if (ret != 0)
        {
            return ret;
        }
        return len > s_len ? 1 : (len < s_len ? -1 : 0);
    }

    uint8_t ByteAt(ssize_t idx) const
    {
        Assert(idx >= 0 && idx < Len());
        return Data()[idx];
    }

    tp_string Hex(bool use_upper_case) const;

    static tp_string Sprintf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
};

#pragma push_macro("DEF_PPGO_BASE_TYPE_NAME_FUNCS")
#undef DEF_PPGO_BASE_TYPE_NAME_FUNCS
#define DEF_PPGO_BASE_TYPE_NAME_FUNCS(_tp) inline std::string TypeName(const tp_##_tp *) { return #_tp; }

DEF_PPGO_BASE_TYPE_NAME_FUNCS(bool)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(byte)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(int)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(uint)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(float)

DEF_PPGO_BASE_TYPE_NAME_FUNCS(i8)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(u8)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(i16)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(u16)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(i32)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(u32)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(i64)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(u64)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(float32)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(float64)

DEF_PPGO_BASE_TYPE_NAME_FUNCS(string)

#undef DEF_PPGO_BASE_TYPE_NAME_FUNCS
#pragma pop_macro("DEF_PPGO_BASE_TYPE_NAME_FUNCS")

class Exc;

class Any : public RCObj
{
public:

    typedef RCPtr<Any> Ptr;

    //R_*: methods for reflect
    virtual std::string R_TypeName() const = 0;

    virtual RCPtr<Exc> method_str(std::tuple<tp_string> &ret)
    {
        std::get<0>(ret) = tp_string::Sprintf(
            "<object of type '%s' at 0x%llX>",
            R_TypeName().c_str(), reinterpret_cast<unsigned long long>(this));
        return nullptr;
    }

    static std::string TypeName()
    {
        return "any";
    }

    static tp_string ToStr(Ptr a)
    {
        if (a)
        {
            std::tuple<tp_string> ret;
            Assert(!a->method_str(ret));
            return std::get<0>(ret);
        }
        return "<nil>";
    }

    static std::string GetTypeName(Ptr a)
    {
        return a ? a->R_TypeName() : "<nil>";
    }
};

template <typename T>
std::string TypeName(RCPtr<T> *)
{
    return T::TypeName();
}

namespace base_type_boxing
{

template <typename T>
class Obj final: public virtual Any
{
    T t_;

    Obj(T t) : t_(t)
    {
    }

    static tp_string ToStr(bool *b)
    {
        return *b ? "true" : "false";
    }

    template <
        typename IT,
        typename std::enable_if_t<std::is_integral_v<IT> && std::is_signed_v<IT>> * = nullptr>
    static tp_string ToStr(IT *p)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%lld", (long long)*p);
        return buf;
    }

    template <
        typename IT,
        typename std::enable_if_t<std::is_integral_v<IT> && std::is_unsigned_v<IT>> * = nullptr>
    static tp_string ToStr(IT *p)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%llu", (unsigned long long)*p);
        return buf;
    }

    template <
        typename FT,
        typename std::enable_if_t<std::is_floating_point_v<FT>> * = nullptr>
    static tp_string ToStr(FT *p)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.17g", (double)*p);
        return buf;
    }

    static tp_string ToStr(tp_string *s)
    {
        return *s;
    }

public:

    T Value() const
    {
        return t_;
    }

    virtual std::string R_TypeName() const override
    {
        return ::ppgo::TypeName(&t_);
    }

    virtual RCPtr<Exc> method_str(std::tuple<tp_string> &ret) override
    {
        std::get<0>(ret) = this->ToStr(&t_);
        return nullptr;
    }

    static Any::Ptr New(T t)
    {
        return new Obj<T>(t);
    }
};

typedef Obj<tp_string> StrObj;

}

template <typename E>
class Vec;

template <typename K, typename V>
class Map;

template <typename E>
std::string TypeName(Vec<E> *)
{
    return std::string("[]") + TypeName((E *)nullptr);
}

template <typename K, typename V>
std::string TypeName(Map<K, V> *)
{
    return std::string("[") + TypeName((K *)nullptr) + "]" + TypeName((V *)nullptr);
}

template <typename E>
class Vec final
{
    struct VecObj final : public virtual Any
    {
        std::vector<E> v_;

        VecObj(std::initializer_list<E> l) : v_(l)
        {
        }

        virtual std::string R_TypeName() const override
        {
            return ::ppgo::TypeName((Vec<E> *)nullptr);
        }
    };

    RCPtr<VecObj> v_;

public:

    Vec() : v_(new VecObj({}))
    {
    }

    Vec(std::initializer_list<E> l) : v_(new VecObj(l))
    {
    }

    Any::Ptr AsAny() const
    {
        return v_.RawPtr();
    }

    void Append(const E &e)
    {
        v_->v_.emplace_back(e);
    }

    void InsertVec(ssize_t idx, Vec<E> es)
    {
        Assert(idx >= 0 && idx <= Len());
        v_->v_.insert(v_->v_.begin() + (size_t)idx, es.v_->v_.begin(), es.v_->v_.end());
    }

    void Insert(ssize_t idx, const E &e)
    {
        Assert(idx >= 0 && idx <= Len());
        v_->v_.insert(v_->v_.begin() + (size_t)idx, e);
    }

    E &GetRef(ssize_t idx) const
    {
        Assert(idx >= 0 && idx < Len());
        return v_->v_[idx];
    }

    E &GetForSet(ssize_t idx) const
    {
        return GetRef(idx);
    }

    E Get(ssize_t idx) const
    {
        return GetRef(idx);
    }

    E Pop(ssize_t idx) const
    {
        Assert(idx >= 0 && idx < Len());
        E e = v_->v_[idx];
        v_->v_.erase(v_->v_.begin() + (size_t)idx);
        return e;
    }

    ssize_t Len() const
    {
        return (ssize_t)v_->v_.size();
    }

    void Resize(ssize_t sz) const
    {
        Assert(sz >= 0 && sz < (1LL << 48));
        v_->v_.resize(sz);
    }

    static bool AssertType(Any *a, Vec<E> &v)
    {
        Assert(a);
        auto vo = dynamic_cast<VecObj *>(a);
        if (vo)
        {
            v.v_ = vo;
            return true;
        }
        return false;
    }
};

template <typename K, typename V>
class Map final
{
    template <typename T>
    struct Less
    {
        bool operator()(T a, T b) const
        {
            return a < b;
        }
    };

    template <typename T>
    struct Less<RCPtr<T>>
    {
        bool operator()(RCPtr<T> a, RCPtr<T> b) const
        {
            std::tuple<tp_int> ret;
            Assert(!a->method_cmp(ret, b));
            return std::get<0>(ret) < 0;
        }
    };

    struct MapObj final : public virtual Any
    {
        std::map<K, V, Less<K>> m_;

        MapObj(std::initializer_list<std::pair<const K, V>> l) : m_(l)
        {
        }

        virtual std::string R_TypeName() const override
        {
            return ::ppgo::TypeName((Map<K, V> *)nullptr);
        }
    };

    RCPtr<MapObj> m_;

public:

    Map() : m_(new MapObj({}))
    {
    }

    Map(std::initializer_list<std::pair<const K, V>> l) : m_(new MapObj(l))
    {
    }

    Any::Ptr AsAny() const
    {
        return m_.RawPtr();
    }

    V &GetRef(K k) const
    {
        auto iter = m_->m_.find(k);
        Assert(iter != m_->m_.end());
        return iter->second;
    }

    V &GetForSet(K k) const
    {
        return m_->m_[k];
    }

    V Get(K k) const
    {
        return GetRef(k);
    }

    static bool AssertType(Any *a, Map<K, V> &m)
    {
        Assert(a);
        auto mo = dynamic_cast<MapObj *>(a);
        if (mo)
        {
            m.m_ = mo;
            return true;
        }
        return false;
    }
};

RCPtr<Exc> NewTypeAssertionException();

template <typename E>
bool _AssertType(Any *a, Vec<E> &v)
{
    return Vec<E>::AssertType(a, v);
}

template <typename K, typename V>
bool _AssertType(Any *a, Map<K, V> &m)
{
    return Map<K, V>::AssertType(a, m);
}

template <typename T>
bool _AssertType(Any *a, RCPtr<T> &t)
{
    Assert(a);
    auto p = dynamic_cast<T *>(a);
    if (p)
    {
        t = p;
        return true;
    }
    return false;
}

template <typename T>
bool _AssertType(Any *a, T &t)
{
    Assert(a);
    auto p = dynamic_cast<base_type_boxing::Obj<T> *>(a);
    if (p)
    {
        t = p->Value();
        return true;
    }
    return false;
}

template <typename T>
RCPtr<Exc> AssertType(Any::Ptr a, T &t)
{
    if (a && _AssertType(a.RawPtr(), t))
    {
        return nullptr;
    }
    return NewTypeAssertionException();
}

}
