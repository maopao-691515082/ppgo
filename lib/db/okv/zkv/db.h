#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

}

template <>
struct NativeAttrs<PPGO_THIS_MOD::cls_DB>
{
    ::lom::ordered_kv::zkv::DB::Ptr db;
};

}

#pragma ppgo undef-THIS_MOD
