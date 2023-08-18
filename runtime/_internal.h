#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <math.h>

#include <atomic>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <tuple>
#include <initializer_list>
#include <type_traits>
#include <memory>
#include <limits>
#include <optional>
#include <chrono>
#include <thread>
#include <mutex>

#include <unistd.h>
#include <sys/errno.h>
#include <sys/time.h>

#include <lom.h>

#ifndef __GNUC__
#   error error: ppgo needs GNUC
#endif

static_assert(CHAR_BIT == 8, "error: ppgo needs 8-bit `char`");
static_assert(std::is_unsigned_v<char8_t>, "error: ppgo needs unsigned `char8_t`");
static_assert(
    sizeof(short) == 2 && sizeof(int) == 4 && sizeof(long) == 8 && sizeof(long long) == 8,
    "error: ppgo needs 16-bit `short`, 32-bit `int` and 64-bit `long` & `long long`");
static_assert(
    sizeof(void *) == 8 && sizeof(size_t) == 8 && sizeof(ssize_t) == 8,
    "error: ppgo needs 64-bit pointer and `size_t` and `ssize_t`");
static_assert(
    sizeof(float) == 4 && sizeof(double) == 8,
    "error: ppgo needs 32-bit `float` and 64-bit `double`");

namespace ppgo
{

static inline void Assert(bool cond)
{
    ::lom::Assert(cond);
}

}
