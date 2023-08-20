#pragma once

namespace ppgo
{

class Exc
{
    std::shared_ptr<Any> throwed_;
    std::vector<tp_string> tb_;

    class NilExc final : public virtual Any
    {
    public:

        virtual std::string R_TypeName() const override
        {
            return "NilException";
        }

        virtual std::shared_ptr<Exc> method_str(std::tuple<tp_string> &ret) override
        {
            const char *s = "throw(nil) called";
            std::get<0>(ret) = tp_string(s, strlen(s));
            return nullptr;
        }
    };

public:

    typedef ExcPtr Ptr;

    void PushTB(const char *file, int line, const char *func)
    {
        tb_.emplace_back(tp_string::Sprintf("File [%s] Line [%d] Func [%s]", file, line, func));
    }

    Any::Ptr Throwed() const
    {
        return throwed_;
    }

    tp_string FormatWithTB() const
    {
        std::string buf;
        buf.append("Exception<");
        buf.append(throwed_->R_TypeName());
        buf.append(">: ");

        {
            std::tuple<tp_string> ret;
            auto &s = std::get<0>(ret);
            auto exc = throwed_->method_str(ret);
            if (exc)
            {
                s = "!!!<UNKNOWN EXC IN METHOD `str`>!!!";
            }
            buf.append(s.Data());
            buf.append("\n");
        }

        for (auto const &s: tb_)
        {
            buf.append("  from ");
            buf.append(s.Data());
            buf.append("\n");
        }

        return tp_string(buf.data(), buf.size());
    }

    static Ptr New(const Any::Ptr &throwed)
    {
        auto e = Ptr(new Exc);
        e->throwed_ = throwed ? throwed : Any::Ptr(new NilExc());
        return e;
    }

    static inline
        __attribute__((always_inline))
        __attribute__((format(gnu_printf, 1, 2)))
        Ptr Sprintf(const char *fmt, ...)
    {
        return New(::ppgo::base_type_boxing::StrObj::New(
            ::ppgo::tp_string::Sprintf(fmt, __builtin_va_arg_pack())));
    }
};

}
