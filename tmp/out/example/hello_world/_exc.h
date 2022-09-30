#pragma once

namespace ppgo
{

class Exc : public RCObj
{
    RCPtr<Any> throwed_;
    std::vector<tp_string> tb_;

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
        buf.append("Exception: <");
        if (throwed_)
        {
            buf.append(throwed_->TypeName());
            buf.append("> ");
            std::tuple<tp_string> ret;
            auto &s = std::get<0>(ret);
            throwed_->func_str(ret);    //ignore exception
            buf.append(s.Data());
            buf.append("\n");
        }
        else
        {
            buf.append("nil>\n");
        }
        for (auto const &s: tb_)
        {
            buf.append("  from ");
            buf.append(s.Data());
            buf.append("\n");
        }
        return tp_string(buf.data(), buf.size());
    }

    static Ptr New(Any *throwed)
    {
        auto e = new Exc;
        e->throwed_ = throwed;
        return e;
    }
};

}
