#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

}

template <>
struct NativeAttrs<PPGO_THIS_MOD::cls_Mutex>
{
    std::mutex lock;
};

}

#pragma ppgo undef-THIS_MOD
