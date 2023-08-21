#pragma once

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

template <typename E>
::ppgo::Exc::Ptr func_valid(::std::tuple<::ppgo::tp_bool> &ret, ::ppgo::VecView<E> vv)
{
    ::std::get<0>(ret) = vv.Valid();
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_resolve(
    ::std::tuple<::ppgo::Vec<E>, ::ppgo::tp_int, ::ppgo::tp_int> &ret, ::ppgo::VecView<E> vv)
{
    vv.Resolve(::std::get<0>(ret), ::std::get<1>(ret), ::std::get<2>(ret));
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_len(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::VecView<E> vv)
{
    ::std::get<0>(ret) = vv.Len();
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_get(::std::tuple<E> &ret, ::ppgo::VecView<E> vv, ::ppgo::tp_int idx)
{
    if (idx < 0 || idx >= vv.Len())
    {
        return ::ppgo::Exc::Sprintf("vector-view.get: index out of range");
    }
    ::std::get<0>(ret) = vv.Get(idx);
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_set(::std::tuple<> &ret, ::ppgo::VecView<E> vv, ::ppgo::tp_int idx, E e)
{
    if (idx < 0 || idx >= vv.Len())
    {
        return ::ppgo::Exc::Sprintf("vector-view.set: index out of range");
    }
    vv.GetForSet(idx) = e;
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
