#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

}

template <>
struct ClsBase<PPGO_THIS_MOD::cls_Conn>
{
    ::lom::fiber::Conn na_conn;

    ~ClsBase()
    {
        na_conn.Close();
    }
};

}

#pragma ppgo undef-THIS_MOD
