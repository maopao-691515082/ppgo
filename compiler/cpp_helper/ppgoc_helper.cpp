#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

extern "C" const char *ParseLongDouble(const char *s)
{
    const char *end_ptr;
    errno = 0;
    auto v = strtold(s, const_cast<char **>(&end_ptr));
    if (*end_ptr == '\0' && errno == 0)
    {
        static char buf[128];
        snprintf(buf, sizeof(buf), "%La", v);
        return buf;
    }
    return nullptr;
}
