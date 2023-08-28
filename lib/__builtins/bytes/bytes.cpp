#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr func_copy_from(
    ::std::tuple<> &ret, ::ppgo::Vec<::ppgo::tp_byte> b, ::ppgo::VecView<::ppgo::tp_byte> from_b)
{
    if (!from_b.Valid())
    {
        ::ppgo::Exc::Sprintf("invalid view");
    }
    auto len = from_b.Len();
    b.Resize(len);
    memcpy(&b.GetRef(0), &from_b.GetRef(0), len);
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
