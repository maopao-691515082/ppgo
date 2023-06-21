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

#include <unistd.h>
#include <sys/errno.h>
#include <sys/time.h>

#include <lom.h>

namespace ppgo
{

static inline void Assert(bool cond)
{
    ::lom::Assert(cond);
}

}
