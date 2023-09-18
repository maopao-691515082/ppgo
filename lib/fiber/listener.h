#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

}

template <>
struct NativeAttrs<PPGO_THIS_MOD::cls_Listener>
{
    ::lom::fiber::Listener listener;

    ~NativeAttrs()
    {
        listener.Close();
    }
};

}

#pragma ppgo undef-THIS_MOD
