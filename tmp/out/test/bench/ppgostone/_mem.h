#pragma once

namespace ppgo
{

#ifdef PPGO_SINGLE_THREADING_MODE

class RC
{
    int64_t rc_;

    RC(const RC &) = delete;
    RC &operator=(const RC &) = delete;

public:

    RC(int64_t rc) : rc_(rc)
    {
    }

    int64_t fetch_add(int64_t n)
    {
        int64_t rc = rc_;
        rc_ += n;
        return rc;
    }

    int64_t load() const
    {
        return rc_;
    }
};

#else

typedef std::atomic<int64_t> RC;

#endif

class RCObj
{
    RC rc_;

    template<class T>
    friend class RCPtr;

protected:

    RCObj() : rc_(0)
    {
    }

    virtual ~RCObj() = 0;
};

template <class T>
class RCPtr
{
    T *p_;

    static void IncRC(T *p)
    {
        if (p != nullptr)
        {
            static_cast<RCObj *>(p)->rc_.fetch_add(1);
        }
    }

    void IncRC() const
    {
        IncRC(p_);
    }

    static void DecRC(T *p)
    {
        if (p != nullptr)
        {
            if (static_cast<RCObj *>(p)->rc_.fetch_add(-1) == 1)
            {
                delete p;
            }
        }
    }

    void DecRC() const
    {
        DecRC(p_);
    }

public:

    RCPtr() : p_(nullptr)
    {
    }

    RCPtr(std::nullptr_t) : p_(nullptr)
    {
    }

    RCPtr(T *p)
    {
        IncRC(p);
        p_ = p;
    }

    RCPtr(const RCPtr<T> &other)
    {
        other.IncRC();
        p_ = other.p_;
    }

    ~RCPtr()
    {
        DecRC();
    }

    RCPtr &operator=(std::nullptr_t)
    {
        Reset();
        return *this;
    }

    RCPtr &operator=(T *p)
    {
        IncRC(p);
        DecRC();
        p_ = p;
        return *this;
    }

    RCPtr &operator=(const RCPtr<T> &other)
    {
        other.IncRC();
        DecRC();
        p_ = other.p_;
        return *this;
    }

    T *operator->() const
    {
        return p_;
    }

    T &operator*() const
    {
        return *p_;
    }

    operator T *() const
    {
        return p_;
    }

    explicit operator bool () const
    {
        return p_ != nullptr;
    }

    bool operator==(const RCPtr<T> &other) const
    {
        return p_ == other.p_;
    }

    bool operator!=(const RCPtr<T> &other) const
    {
        return p_ != other.p_;
    }

    T *RawPtr() const
    {
        return p_;
    }

    int64_t RC() const
    {
        return p_ == nullptr ? 0 : static_cast<RCObj *>(p_)->rc_.load();
    }

    RCPtr<T> Copy() const
    {
        return p_;
    }

    void Reset()
    {
        DecRC();
        p_ = nullptr;
    }
};

}
