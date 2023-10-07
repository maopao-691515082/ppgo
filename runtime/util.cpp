#include "ppgo.h"

namespace ppgo
{

namespace util
{

void OutputUnexpectedErrMsg(const ::lom::Str &s)
{
    auto write_to_stderr = [] (const char *p, ssize_t sz) {
        ::ppgo::Assert(sz >= 0);
        ::lom::io::fds::WriteAll(2, p, sz);
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
