#include "ppgo.h"

namespace ppgo
{

namespace util
{

int64_t NowUS()
{
    struct timeval now;
    gettimeofday(&now, nullptr);
    return (int64_t)now.tv_sec * 1000000 + (int64_t)now.tv_usec;
}

}

}
