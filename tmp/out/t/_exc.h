#pragma once

namespace ppgo
{

class Exc : public RCObj
{
    RCPtr<Any> throwed_;
    std::vector<tp_string> tb_;

    class NilExc final : public virtual Any
    {
    public:

        virtual std::string R_TypeName() const override
        {
            return "NilException";
        }

        virtual RCPtr<Exc> method_str(std::tuple<tp_string> &ret) override
        {
            const char *s = "throw(nil) called";
            std::get<0>(ret) = tp_string(s, strlen(s));
            return nullptr;
        }
    };

public:

    typedef RCPtr<Exc> Ptr;

    void PushTB(const char *file, int line, const char *func)
    {
        tb_.emplace_back(tp_string::Sprintf("File [%s] Line [%d] Func [%s]", file, line, func));
    }

    RCPtr<Any> Throwed() const
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
            throwed_->method_str(ret);    //ignore exception
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

    static Ptr New(RCPtr<Any> throwed)
    {
        if (!throwed)
        {
            throwed = new NilExc();
        }
        auto e = new Exc;
        e->throwed_ = throwed;
        return e;
    }
};

}
