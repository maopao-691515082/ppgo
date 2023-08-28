#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_len(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_string s)
{
    std::get<0>(ret) = s.Len();
    return nullptr;
}

::ppgo::Exc::Ptr func_repr(::std::tuple<::ppgo::tp_string> &ret, ::ppgo::tp_string s)
{
    std::get<0>(ret) = ::ppgo::tp_string(s.RawStr().Repr());
    return nullptr;
}

::ppgo::Exc::Ptr func_upper(::std::tuple<::ppgo::tp_string> &ret, ::ppgo::tp_string s)
{
    std::get<0>(ret) = ::ppgo::tp_string(s.RawStr().Upper());
    return nullptr;
}

::ppgo::Exc::Ptr func_lower(::std::tuple<::ppgo::tp_string> &ret, ::ppgo::tp_string s)
{
    std::get<0>(ret) = ::ppgo::tp_string(s.RawStr().Lower());
    return nullptr;
}

#define CHECK_BASE() do {                               \
    if (!(base == 0 || (base >= 2 && base <= 36))) {    \
        return ::ppgo::Exc::Sprintf("invalid base");    \
    }                                                   \
} while (false)

#define PARSE_NUM(_e) do {                          \
    auto p = s.Data();                              \
    auto len = s.Len();                             \
    if (len > 0 && !isspace(*p)) {                  \
        const char *end_ptr;                        \
        errno = 0;                                  \
        auto n = (_e);                              \
        if (end_ptr == p + len && errno == 0) {     \
            ::std::get<0>(ret) = n;                 \
            return nullptr;                         \
        }                                           \
    }                                               \
    return ::ppgo::Exc::Sprintf("parse failed");    \
} while (false)

::ppgo::Exc::Ptr func_parse_int(
    ::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_string s, std::optional<::ppgo::tp_int> opt_base)
{
    ::ppgo::tp_int base = opt_base.value_or(0);
    CHECK_BASE();
    PARSE_NUM(strtoll(p, const_cast<char **>(&end_ptr), base));
}

::ppgo::Exc::Ptr func_parse_uint_with_base(
    ::std::tuple<::ppgo::tp_uint> &ret, ::ppgo::tp_string s, std::optional<::ppgo::tp_int> opt_base)
{
    ::ppgo::tp_int base = opt_base.value_or(0);
    CHECK_BASE();
    PARSE_NUM(strtoull(p, const_cast<char **>(&end_ptr), base));
}

::ppgo::Exc::Ptr func_parse_float(::std::tuple<::ppgo::tp_float> &ret, ::ppgo::tp_string s)
{
    PARSE_NUM(strtold(p, const_cast<char **>(&end_ptr)));
}

::ppgo::Exc::Ptr func_contains_byte(
    ::std::tuple<::ppgo::tp_bool> &ret, ::ppgo::tp_string s, ::ppgo::tp_byte b)
{
    std::get<0>(ret) = s.RawStr().ContainsByte(b);
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
