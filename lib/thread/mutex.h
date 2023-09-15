#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

}

template <>
struct ClsBase<PPGO_THIS_MOD::cls_Mutex>
{
    std::mutex na_l;
};

}

#pragma ppgo undef-THIS_MOD
