#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

}

template <>
struct NativeAttrs<PPGO_THIS_MOD::cls_Conn>
{
    ::lom::fiber::Conn conn;

    ~NativeAttrs()
    {
        conn.Close();
    }
};

}

#pragma ppgo undef-THIS_MOD
