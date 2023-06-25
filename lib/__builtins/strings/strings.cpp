#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

#define CHECK_BASE() do {                                               \
    if (base < 0 || base == 1 || base > 36) {                           \
        return ::ppgo::Exc::New(::ppgo::base_type_boxing::StrObj::New(  \
            ::ppgo::tp_string("invalid base")));                        \
    }                                                                   \
} while (false)

#define PARSE_NUM(_e) do {                                                                              \
    auto p = s.Data();                                                                                  \
    auto len = s.Len();                                                                                 \
    if (len > 0 && !isspace(*p)) {                                                                      \
        const char *end_ptr;                                                                            \
        errno = 0;                                                                                      \
        auto n = (_e);                                                                                  \
        if (end_ptr == p + len && errno == 0) {                                                         \
            ::std::get<0>(ret) = n;                                                                     \
            return nullptr;                                                                             \
        }                                                                                               \
    }                                                                                                   \
    return ::ppgo::Exc::New(::ppgo::base_type_boxing::StrObj::New(::ppgo::tp_string("parse failed")));  \
} while (false)

::ppgo::Exc::Ptr func_parse_int_with_base(
    ::std::tuple<::ppgo::tp_int> &ret, const ::ppgo::tp_string &s, ::ppgo::tp_int base)
{
    CHECK_BASE();
    PARSE_NUM(strtoll(p, const_cast<char **>(&end_ptr), base));
}

::ppgo::Exc::Ptr func_parse_uint_with_base(
    ::std::tuple<::ppgo::tp_uint> &ret, const ::ppgo::tp_string &s, ::ppgo::tp_int base)
{
    CHECK_BASE();
    PARSE_NUM(strtoull(p, const_cast<char **>(&end_ptr), base));
}

::ppgo::Exc::Ptr func_parse_float(::std::tuple<::ppgo::tp_float> &ret, const ::ppgo::tp_string &s)
{
    PARSE_NUM(strtod(p, const_cast<char **>(&end_ptr)));
}

}

}

#pragma ppgo undef-THIS_MOD
