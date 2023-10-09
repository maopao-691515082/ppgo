#pragma once

namespace ppgo
{

class Exc
{
public:

    typedef ExcPtr Ptr;

private:

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

    class LomErrWrapper : public ::lom::Err
    {
        ::ppgo::Exc::Ptr exc_;

    public:

        LomErrWrapper(const ::ppgo::Exc::Ptr &exc) : exc_(exc)
        {
        }

        ::ppgo::Exc::Ptr Get() const
        {
            return exc_;
        }

        virtual ::lom::Str Msg() const override
        {
            return "lom error wrapper of ppgo exception";
        }
    };

public:

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
        bool is_rethrow = tb_.size() >= 2 && tb_.at(0) == "";

        std::string buf;
        if (is_rethrow)
        {
            //rethrow
            buf.append(tb_.at(1).Data());
        }
        else
        {
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
        }

        for (size_t i = is_rethrow ? 2 : 0; i < tb_.size(); ++ i)
        {
            if (tb_.at(i) == "")
            {
                if (i + 1 < tb_.size())
                {
                    buf.append("  rethrowed\n");
                    ++ i;
                }
                else
                {
                    break;
                }
            }
            buf.append("  from ");
            buf.append(tb_.at(i).Data());
            buf.append("\n");
        }

        return tp_string(buf.data(), buf.size());
    }

    static LOM_ERR WrapToLomErr(const Ptr &exc)
    {
        Assert(static_cast<bool>(exc));
        return LOM_ERR(new LomErrWrapper(exc));
    }
    static Ptr FromLomErr(LOM_ERR err)
    {
        Assert(static_cast<bool>(err));
        auto lom_err_wrapper = dynamic_cast<LomErrWrapper *>(err.get());
        return lom_err_wrapper ? lom_err_wrapper->Get() : ::ppgo::Exc::Sprintf("%s", err->Msg().CStr());
    }

    static Ptr New(const Any::Ptr &throwed)
    {
        auto e = Ptr(new Exc);
        e->throwed_ = throwed ? throwed : Any::Ptr(new NilExc());
        return e;
    }

    static Ptr NewRethrow(const Any::Ptr &throwed, const ::ppgo::tp_string &tb)
    {
        auto e = Ptr(new Exc);
        e->throwed_ = throwed ? throwed : Any::Ptr(new NilExc());
        e->tb_.emplace_back("");
        e->tb_.emplace_back(tb);
        e->tb_.emplace_back("");
        return e;
    }

    [[gnu::always_inline]] [[gnu::format(gnu_printf, 1, 2)]]
    static inline Ptr Sprintf(const char *fmt, ...)
    {
        return New(::ppgo::base_type_boxing::StrObj::New(
            ::ppgo::tp_string::Sprintf(fmt, __builtin_va_arg_pack())));
    }
};

}
