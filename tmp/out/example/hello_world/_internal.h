#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#include <atomic>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <functional>
#include <tuple>

#include <unistd.h>
#include <sys/errno.h>

#ifndef __GNUC__
#   error error: ppgo needs GNUC
#endif

static_assert(sizeof(long long) == 8, "error: ppgo needs 64-bit `long long`");
static_assert(
    sizeof(void *) == 8 && sizeof(size_t) == 8 && sizeof(ssize_t) == 8,
    "error: ppgo needs 64-bit pointer and `size_t` and `ssize_t`");
static_assert(
    sizeof(float) == 4 && sizeof(double) == 8,
    "error: lom needs 32-bit `float` and 64-bit `double`");


namespace ppgo
{

static inline void Assert(bool cond)
{
    if (!cond)
    {
        abort();
    }
}

}
