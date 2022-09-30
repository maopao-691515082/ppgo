#include "ppgo.h"

#ifdef PPGO_THIS_MOD
#error macro PPGO_THIS_MOD redefined
#endif
#define PPGO_THIS_MOD mod6D07

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_println(::std::tuple<> &ret, ::ppgo::tp_string s)
{
    auto write_to_stdout = [] (const char *p, ssize_t sz) -> ::ppgo::Exc::Ptr {
        ::ppgo::Assert(sz >= 0);
        while (sz > 0)
        {
            ssize_t n = ::write(1, p, (size_t)sz);
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

    return write_to_stdout(s.Data(), s.Len()) ?: write_to_stdout("\n", 1);
}

}

}

#undef PPGO_THIS_MOD
