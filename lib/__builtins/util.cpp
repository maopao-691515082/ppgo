#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_print_to_std(
    ::std::tuple<> &ret, ::ppgo::Any::Ptr a, ::ppgo::tp_bool is_println, ::ppgo::tp_bool to_std_err)
{
    auto write_to_stdout = [to_std_err] (const char *p, ssize_t sz) {
        ::ppgo::Assert(sz >= 0);
        while (sz > 0)
        {
            ssize_t n = ::write(to_std_err ? 2 : 1, p, static_cast<size_t>(sz));
            if (n == -1)
            {
                return;
            }
            ::ppgo::Assert(n <= sz);
            sz -= n;
        }
    };

    auto s = ::ppgo::Any::ToStr(a);
    write_to_stdout(s.Data(), s.Len());
    if (is_println)
    {
        write_to_stdout("\n", 1);
    }
    return nullptr;
}

::ppgo::Exc::Ptr func_assert(::std::tuple<> &ret, ::ppgo::tp_bool cond)
{
    ::ppgo::Assert(cond);
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
