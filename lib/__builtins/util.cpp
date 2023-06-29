#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_print_to_std(
    ::std::tuple<> &ret, ::ppgo::Any::Ptr a, ::ppgo::tp_bool is_println, ::ppgo::tp_bool to_std_err)
{
    auto write_to_stdout = [to_std_err] (const char *p, ssize_t sz) -> ::ppgo::Exc::Ptr {
        ::ppgo::Assert(sz >= 0);
        while (sz > 0)
        {
            ssize_t n = ::write(to_std_err ? 2 : 1, p, static_cast<size_t>(sz));
            if (n == -1)
            {
                return ::ppgo::Exc::New(::ppgo::base_type_boxing::StrObj::New(
                    ::ppgo::tp_string::Sprintf("IO error, errno [%d]", errno)));
            }
            ::ppgo::Assert(n <= sz);
            sz -= n;
        }
        return nullptr;
    };

    auto s = ::ppgo::Any::ToStr(a);
    return write_to_stdout(s.Data(), s.Len()) ?: (is_println ? write_to_stdout("\n", 1) : nullptr);
}

::ppgo::Exc::Ptr func_assert(::std::tuple<> &ret, ::ppgo::tp_bool cond)
{
    ::ppgo::Assert(cond);
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
