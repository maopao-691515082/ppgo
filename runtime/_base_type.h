#pragma once

namespace ppgo
{

//C类型的位数已经在`_internal.h`做了保证

typedef bool                tp_bool;
typedef char8_t             tp_byte;
typedef long long           tp_int;
typedef unsigned long long  tp_uint;
typedef long double         tp_float;

typedef signed char     tp_i8;
typedef unsigned char   tp_u8;
typedef short           tp_i16;
typedef unsigned short  tp_u16;
typedef int             tp_i32;
typedef unsigned int    tp_u32;
typedef long            tp_i64;
typedef unsigned long   tp_u64;

typedef float   tp_f32;
typedef double  tp_f64;

typedef void *tp_uptr;

class tp_string final
{
    ::lom::Str s_;

    tp_string(std::nullptr_t) = delete;

public:

    tp_string(const char *p, ssize_t len) : s_(p, len)
    {
    }

    tp_string() : tp_string("", 0)
    {
    }

    tp_string(const char *s) : tp_string(s, strlen(s))
    {
    }

    tp_string(const ::lom::Str &s) : s_(s)
    {
    }

    tp_string(::lom::Str &&s) : s_(std::move(s))
    {
    }

    const ::lom::Str &RawStr() const
    {
        return s_;
    }

    const char *Data() const
    {
        return s_.Data();
    }
    ssize_t Len() const
    {
        return s_.Len();
    }

    int Cmp(const tp_string &s) const
    {
        return s_.Cmp(s.s_);
    }

    tp_byte ByteAt(ssize_t idx) const
    {
        Assert(idx >= 0 && idx < Len());
        return static_cast<tp_byte>(Data()[idx]);
    }

    tp_string Hex(bool upper_case) const
    {
        return tp_string(s_.Hex(upper_case));
    }

    tp_string Concat(const tp_string &s) const
    {
        return tp_string(s_.Concat(s.s_));
    }

    static inline
        __attribute__((always_inline))
        __attribute__((format(gnu_printf, 1, 2)))
        tp_string Sprintf(const char *fmt, ...)
    {
        return tp_string(::lom::Sprintf(fmt, __builtin_va_arg_pack()));
    }
};

#pragma push_macro("DEF_PPGO_BASE_TYPE_NAME_FUNCS")
#undef DEF_PPGO_BASE_TYPE_NAME_FUNCS

#define DEF_PPGO_BASE_TYPE_NAME_FUNCS(_tp)          \
    inline std::string TypeName(const tp_##_tp *) { \
        return #_tp;                                \
    }

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
DEF_PPGO_BASE_TYPE_NAME_FUNCS(f32)
DEF_PPGO_BASE_TYPE_NAME_FUNCS(f64)

DEF_PPGO_BASE_TYPE_NAME_FUNCS(uptr)

DEF_PPGO_BASE_TYPE_NAME_FUNCS(string)

#pragma pop_macro("DEF_PPGO_BASE_TYPE_NAME_FUNCS")

class Exc;
typedef std::shared_ptr<Exc> ExcPtr;

class Any
{
public:

    typedef std::shared_ptr<Any> Ptr;

    virtual ~Any()
    {
    }

    //R_*: methods for reflect
    virtual std::string R_TypeName() const = 0;

    virtual ExcPtr method_str(std::tuple<tp_string> &ret)
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
std::string TypeName(const std::shared_ptr<T> *)
{
    return T::TypeName();
}

namespace base_type_boxing
{

template <typename T>
class Obj final: public virtual Any
{
    T t_;

    Obj(const T &t) : t_(t)
    {
    }

    static tp_string ToStr(const bool *b)
    {
        return *b ? "true" : "false";
    }

    template <
        typename IT,
        typename std::enable_if_t<std::is_integral_v<IT> && std::is_signed_v<IT>> * = nullptr>
    static tp_string ToStr(const IT *p)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(*p));
        return buf;
    }

    template <
        typename IT,
        typename std::enable_if_t<std::is_integral_v<IT> && std::is_unsigned_v<IT>> * = nullptr>
    static tp_string ToStr(const IT *p)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%llu", static_cast<unsigned long long>(*p));
        return buf;
    }

    template <
        typename FT,
        typename std::enable_if_t<std::is_floating_point_v<FT>> * = nullptr>
    static tp_string ToStr(const FT *p)
    {
        char buf[128];
        snprintf(
            buf, sizeof(buf),
            "%.*Lg", std::numeric_limits<long double>::max_digits10, static_cast<long double>(*p));
        return buf;
    }

    static tp_string ToStr(const tp_uptr *p)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "<@%p>", *p);
        return buf;
    }

    static tp_string ToStr(const tp_string *s)
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

    virtual ExcPtr method_str(std::tuple<tp_string> &ret) override
    {
        std::get<0>(ret) = this->ToStr(&t_);
        return nullptr;
    }

    static Any::Ptr New(const T &t)
    {
        return Any::Ptr(new Obj<T>(t));
    }
};

typedef Obj<tp_string> StrObj;

}

template <typename E>
class Vec;

template <typename K, typename V>
class Map;

template <typename E>
class Set;

template <typename E>
std::string TypeName(const Vec<E> *)
{
    return std::string("[]") + TypeName(static_cast<const E *>(nullptr));
}

template <typename K, typename V>
std::string TypeName(const Map<K, V> *)
{
    return (
        std::string("[") + TypeName(static_cast<const K *>(nullptr)) + "]" +
        TypeName(static_cast<const V *>(nullptr)));
}

template <typename E>
std::string TypeName(const Set<E> *)
{
    return std::string("[") + TypeName(static_cast<const E *>(nullptr)) + "]_";
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
            return ::ppgo::TypeName(static_cast<const Vec<E> *>(nullptr));
        }
    };

    std::shared_ptr<VecObj> v_;

public:

    Vec() : v_(new VecObj({}))
    {
    }

    Vec(std::initializer_list<E> l) : v_(new VecObj(l))
    {
    }

    Any::Ptr AsAny() const
    {
        return std::static_pointer_cast<Any>(v_);
    }

    void Append(E e) const
    {
        v_->v_.emplace_back(e);
    }

    void InsertVec(ssize_t idx, Vec<E> es) const
    {
        Assert(idx >= 0 && idx <= Len());
        if (v_ != es.v_)
        {
            v_->v_.insert(v_->v_.begin() + static_cast<size_t>(idx), es.v_->v_.begin(), es.v_->v_.end());
            return;
        }

        //es就是自身，走特殊流程
        std::vector<E> v(es.v_->v_);
        v_->v_.insert(v_->v_.begin() + static_cast<size_t>(idx), v.begin(), v.end());
    }

    void Insert(ssize_t idx, E e) const
    {
        Assert(idx >= 0 && idx <= Len());
        v_->v_.insert(v_->v_.begin() + static_cast<size_t>(idx), e);
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
        v_->v_.erase(v_->v_.begin() + static_cast<size_t>(idx));
        return e;
    }

    ssize_t Len() const
    {
        return static_cast<ssize_t>(v_->v_.size());
    }

    void Resize(ssize_t sz) const
    {
        Assert(sz >= 0 && sz < (1LL << 48));
        v_->v_.resize(static_cast<size_t>(sz));
    }

    static bool AssertType(const Any::Ptr &a, Vec<E> &v)
    {
        Assert(static_cast<bool>(a));
        auto vo = std::dynamic_pointer_cast<VecObj>(a);
        if (vo)
        {
            v.v_ = vo;
            return true;
        }
        return false;
    }
};

template <typename T>
struct Less
{
    bool operator()(const T &a, const T &b) const
    {
        return a < b;
    }
};

template <typename T>
struct Less<std::shared_ptr<T>>
{
    bool operator()(std::shared_ptr<T> a, std::shared_ptr<T> b) const
    {
        std::tuple<tp_int> ret;
        Assert(!a->method_cmp(ret, b));
        return std::get<0>(ret) < 0;
    }
};

template <typename K, typename V>
class Map final
{
    struct MapObj final : public virtual Any
    {
        typedef std::map<K, V, Less<K>> map_t;

        map_t m_;

        MapObj(std::initializer_list<std::pair<const K, V>> l) : m_(l)
        {
        }

        virtual std::string R_TypeName() const override
        {
            return ::ppgo::TypeName(static_cast<const Map<K, V> *>(nullptr));
        }
    };

    std::shared_ptr<MapObj> m_;

public:

    Map() : m_(new MapObj({}))
    {
    }

    Map(std::initializer_list<std::pair<const K, V>> l) : m_(new MapObj(l))
    {
    }

    Any::Ptr AsAny() const
    {
        return std::static_pointer_cast<Any>(m_);
    }

    ssize_t Len() const
    {
        return (ssize_t)m_->m_.size();
    }

    V &GetRef(const K &k) const
    {
        auto iter = m_->m_.find(k);
        Assert(iter != m_->m_.end());
        return iter->second;
    }

    V &GetForSet(const K &k) const
    {
        return m_->m_[k];
    }

    V Get(const K &k) const
    {
        return GetRef(k);
    }

    bool GetOrPop(const K &k, V *v = nullptr, bool need_pop = false) const
    {
        auto iter = m_->m_.find(k);
        if (iter == m_->m_.end())
        {
            return false;
        }
        if (v)
        {
            *v = iter->second;
        }
        if (need_pop)
        {
            m_->m_.erase(iter);
        }
        return true;
    }

    class Iter
    {
        std::shared_ptr<MapObj> m_;
        typename MapObj::map_t::const_iterator it_, end_;

    public:

        Iter(Map<K, V> m) : m_(m.m_), it_(m.m_->m_.begin()), end_(m.m_->m_.end())
        {
        }

        bool Valid() const
        {
            return it_ != end_;
        }

        void Inc()
        {
            ++ it_;
        }

        K Key() const
        {
            return it_->first;
        }

        V Value() const
        {
            return it_->second;
        }
    };

    Iter NewIter()
    {
        return Iter(*this);
    }

    static bool AssertType(const Any::Ptr &a, Map<K, V> &m)
    {
        Assert(static_cast<bool>(a));
        auto mo = std::dynamic_pointer_cast<MapObj>(a);
        if (mo)
        {
            m.m_ = mo;
            return true;
        }
        return false;
    }
};

template <typename E>
class Set final
{
    struct SetObj final : public virtual Any
    {
        typedef std::set<E, Less<E>> set_t;

        set_t s_;

        SetObj(std::initializer_list<E> l) : s_(l)
        {
        }

        virtual std::string R_TypeName() const override
        {
            return ::ppgo::TypeName(static_cast<const Set<E> *>(nullptr));
        }
    };

    std::shared_ptr<SetObj> s_;

public:

    Set() : s_(new SetObj({}))
    {
    }

    Set(std::initializer_list<E> l) : s_(new SetObj(l))
    {
    }

    Any::Ptr AsAny() const
    {
        return std::static_pointer_cast<Any>(s_);
    }

    ssize_t Len() const
    {
        return static_cast<ssize_t>(s_->s_.size());
    }

    bool Has(const E &e) const
    {
        return s_->s_.count(e) > 0;
    }

    void Add(E e) const
    {
        s_->s_.emplace(e);
    }

    void Remove(E e) const
    {
        s_->s_.erase(e);
    }

    class Iter
    {
        std::shared_ptr<SetObj> s_;
        typename SetObj::set_t::const_iterator it_, end_;

    public:

        Iter(Set<E> s) : s_(s.s_), it_(s.s_->s_.begin()), end_(s.s_->s_.end())
        {
        }

        bool Valid() const
        {
            return it_ != end_;
        }

        void Inc()
        {
            ++ it_;
        }

        E Elem() const
        {
            return *it_;
        }
    };

    Iter NewIter()
    {
        return Iter(*this);
    }

    static bool AssertType(const Any::Ptr &a, Set<E> &s)
    {
        Assert(static_cast<bool>(a));
        auto so = std::dynamic_pointer_cast<SetObj>(a);
        if (so)
        {
            s.s_ = so;
            return true;
        }
        return false;
    }
};

ExcPtr NewTypeAssertionException();

template <typename E>
bool _AssertType(const Any::Ptr &a, Vec<E> &v)
{
    return Vec<E>::AssertType(a, v);
}

template <typename K, typename V>
bool _AssertType(const Any::Ptr &a, Map<K, V> &m)
{
    return Map<K, V>::AssertType(a, m);
}

template <typename E>
bool _AssertType(const Any::Ptr &a, Set<E> &s)
{
    return Set<E>::AssertType(a, s);
}

template <typename T>
bool _AssertType(const Any::Ptr &a, std::shared_ptr<T> &t)
{
    Assert(static_cast<bool>(a));
    auto p = std::dynamic_pointer_cast<T>(a);
    if (p)
    {
        t = p;
        return true;
    }
    return false;
}

template <typename T>
bool _AssertType(const Any::Ptr &a, T &t)
{
    Assert(static_cast<bool>(a));
    auto p = std::dynamic_pointer_cast<base_type_boxing::Obj<T>>(a);
    if (p)
    {
        t = p->Value();
        return true;
    }
    return false;
}

template <typename T>
ExcPtr AssertType(const Any::Ptr &a, T &t)
{
    if (a && _AssertType(a, t))
    {
        return nullptr;
    }
    return NewTypeAssertionException();
}

}
