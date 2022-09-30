#include "ppgo.h"

namespace ppgo
{

static const ssize_t kShortStrLenMax = sizeof(tp_string) - 2;   //1-byte len + 1-byte nul end

void tp_string::AssignLongStr(const char *p, ssize_t len)
{
    Assert(len < ((ssize_t)1 << 48));

    ss_len_ = -1;
    ls_len_high_ = len >> 32;
    ls_len_low_ = len & ~(uint32_t)0;
    lsp_ = new LongStr(p);
}

tp_string::tp_string(const char *data, ssize_t len)
{
    if (len <= kShortStrLenMax)
    {
        ss_len_ = len;
        char *ss = &ss_start_;
        memcpy(ss, data, len);
        ss[len] = '\0';
        return;
    }

    char *p = new char [len + 1];
    memcpy(p, data, len);
    p[len] = '\0';
    AssignLongStr(p, len);
}

tp_string tp_string::Sprintf(const char *fmt, ...)
{
    static thread_local char buf[128];
    int need_len;
    {
        va_list ap;
        va_start(ap, fmt);
        need_len = vsnprintf(buf, sizeof(buf), fmt, ap);
        Assert(need_len >= 0);
        va_end(ap);
        if (need_len < (int)sizeof(buf))
        {
            return tp_string(buf, need_len);
        }
    }
    {
        char *p = new char [(size_t)need_len + 1];
        va_list ap;
        va_start(ap, fmt);
        Assert(vsnprintf(p, (size_t)need_len + 1, fmt, ap) == need_len && p[need_len] == '\0');
        va_end(ap);

        tp_string s;
        s.Destruct();
        s.AssignLongStr(p, need_len);
        return s;
    }
}

}
