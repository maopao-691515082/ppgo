#include "ppgo.h"

namespace ppgo
{

namespace util
{

void OutputUnexpectedErrMsg(const ::lom::Str &s)
{
    auto write_to_stderr = [] (const char *p, ssize_t sz) {
        ::ppgo::Assert(sz >= 0);
        while (sz > 0)
        {
            ssize_t n = ::write(2, p, static_cast<size_t>(sz));
            if (n == -1)
            {
                return;
            }
            ::ppgo::Assert(n <= sz);
            sz -= n;
        }
    };

    write_to_stderr(s.Data(), s.Len());
    write_to_stderr("\n", 1);
}

}

tp_string ExcFormatWithTB(const ExcPtr exc)
{
    return exc->FormatWithTB();
}

}
