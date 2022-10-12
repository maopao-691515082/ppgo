#pragma once

namespace ppgo
{

template <typename W>
class WithGuard final
{
    W w_;
    Exc::Ptr exc_;

public:

    WithGuard(W w) : w_(w)
    {
        std::tuple<> ret;
        exc_ = w_->method_enter(ret);
    }

    ~WithGuard()
    {
        if (exc_)
        {
            return;
        }

        std::tuple<> ret;
        auto exc = w_->method_exit(ret);
        if (exc)
        {
            auto ftb = exc->FormatWithTB();
            fprintf(stderr, "Uncached exception in with-exit %s\n", ftb.Data());
        }
    }

    Exc::Ptr ExcOfEnter() const
    {
        return exc_;
    }
};

}
