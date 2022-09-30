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
    char buf[128];
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
        char *p = new char[(size_t)need_len + 1];
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

tp_string tp_string::Hex(bool use_upper_case) const
{
    auto data = Data();
    auto len = Len();
    auto hex_digests = use_upper_case ? "0123456789ABCDEF" : "0123456789abcdef";
    auto output = [&] (char *buf) {
        for (ssize_t i = 0; i < len; ++ i)
        {
            auto uc = (unsigned char)data[i];
            buf[i * 2] = hex_digests[uc / 16];
            buf[i * 2 + 1] = hex_digests[uc % 16];
        }
    };

    auto need_len = Len() * 2;
    if (need_len <= kShortStrLenMax)
    {
        char buf[need_len];
        output(buf);
        return tp_string(buf, need_len);
    }

    char *p = new char[need_len + 1];
    output(p);
    p[need_len] = '\0';
    tp_string s;
    s.Destruct();
    s.AssignLongStr(p, need_len);
    return s;
}

}
