#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

class RandGenerator
{
    uint64_t seed_x_, seed_y_, seed_z_;

public:

    RandGenerator(uint64_t seed)
    {
        seed_x_ = seed % 30268;
        seed /= 30268;
        seed_y_ = seed % 30306;
        seed /= 30306;
        seed_z_ = seed % 30322;
        seed /= 30322;
        ++ seed_x_;
        ++ seed_y_;
        ++ seed_z_;
    }

    double Rand()
    {
        seed_x_ = seed_x_ * 171 % 30269;
        seed_y_ = seed_y_ * 172 % 30307;
        seed_z_ = seed_z_ * 170 % 30323;
        return fmod(seed_x_ / 30269.0 + seed_y_ / 30307.0 + seed_z_ / 30323.0, 1.0);
    }
};

static RandGenerator *TLSRandGenerator()
{
    static thread_local RandGenerator *rand_generator = nullptr;
    if (rand_generator == nullptr)
    {
        rand_generator = new RandGenerator((uint64_t)::ppgo::util::NowUS());
    }
    return rand_generator;
}

::ppgo::Exc::Ptr func_rand(::std::tuple<::ppgo::tp_float> &ret)
{
    ::std::get<0>(ret) = TLSRandGenerator()->Rand();
    return nullptr;
}

::ppgo::Exc::Ptr func_fast_rand_n(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int n)
{
    ::ppgo::Assert(n > 0);
    static thread_local ::ppgo::tp_uint r = (::ppgo::tp_uint)::ppgo::util::NowUS();
    r = r * 1000003 + 1;
    ::std::get<0>(ret) = (::ppgo::tp_int)(r % (::ppgo::tp_uint)n);
    return nullptr;
}

}

}
