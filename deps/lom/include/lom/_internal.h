#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <ctype.h>
#include <malloc.h>
#include <math.h>

#include <vector>
#include <initializer_list>
#include <utility>
#include <functional>
#include <type_traits>
#include <list>
#include <map>
#include <atomic>
#include <string>
#include <memory>
#include <random>
#include <limits>
#include <thread>
#include <mutex>
#include <optional>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef __GNUC__
#   error error: lom needs GNUC
#endif

static_assert(CHAR_BIT == 8, "error: lom needs 8-bit `char`");
static_assert(
    sizeof(short) == 2 && sizeof(int) == 4 && sizeof(long) == 8 && sizeof(long long) == 8,
    "error: lom needs 16-bit `short`, 32-bit `int` and 64-bit `long` & `long long`");
static_assert(
    sizeof(void *) == 8 && sizeof(size_t) == 8 && sizeof(ssize_t) == 8,
    "error: lom needs 64-bit pointer and `size_t` and `ssize_t`");
static_assert(
    sizeof(float) == 4 && sizeof(double) == 8,
    "error: lom needs 32-bit `float` and 64-bit `double`");
static_assert(
    std::numeric_limits<float>::is_iec559 && std::numeric_limits<double>::is_iec559 &&
    std::numeric_limits<long double>::is_iec559,
    "error: lom needs IEEE-754 floating-point arithmetic");
static_assert(
    sizeof(off_t) == 8 && std::is_signed_v<off_t>,
    "error: lom needs 64-bit signed `off_t`");

//先行声明错误类型
#define LOM_ERR ::lom::NoDiscard<::std::shared_ptr<::lom::Err>>
namespace lom { class Err; }

namespace lom
{

//将类`T`扩展为nodiscard版本
template <typename T>
class [[nodiscard]] NoDiscard : public T
{
public:

    //直接`using`继承构造函数可能存在一些不匹配的情况，用更通用的模板方式
    template <typename ...Args>
    NoDiscard(Args &&...args) : T(std::forward<Args>(args)...)
    {
    }
};

static inline void Assert(bool cond)
{
    if (!cond)
    {
        abort();
    }
}

}

#define LOM_DISCARD(_expr) do { \
    static_cast<void>(_expr);   \
} while (false)
