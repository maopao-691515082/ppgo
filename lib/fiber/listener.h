#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

}

template <>
struct ClsBase<PPGO_THIS_MOD::cls_Listener>
{
    ::lom::fiber::Listener na_listener;

    ~ClsBase()
    {
        na_listener.Close();
    }
};

}

#pragma ppgo undef-THIS_MOD
