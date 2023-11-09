#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

}

template <>
struct NativeAttrs<PPGO_THIS_MOD::cls_WriteBatch>
{
    ::lom::ordered_kv::WriteBatch wb;
};

template <>
struct NativeAttrs<PPGO_THIS_MOD::cls_Snapshot>
{
    ::lom::ordered_kv::Snapshot::Ptr s;
};

template <>
struct NativeAttrs<PPGO_THIS_MOD::cls_Iter>
{
    ::lom::ordered_kv::Iterator::Ptr iter;
};

}

#pragma ppgo undef-THIS_MOD
