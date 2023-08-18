#pragma once

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

template <typename E>
::ppgo::Exc::Ptr func_len(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::Set<E> s)
{
    ::std::get<0>(ret) = s.Len();
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_has(::std::tuple<::ppgo::tp_bool> &ret, ::ppgo::Set<E> s, E e)
{
    ::std::get<0>(ret) = s.Has(e);
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_add(::std::tuple<::ppgo::Set<E>> &ret, ::ppgo::Set<E> s, E e)
{
    s.Add(e);
    ::std::get<0>(ret) = s;
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_remove(::std::tuple<::ppgo::Set<E>> &ret, ::ppgo::Set<E> s, E e)
{
    s.Remove(e);
    ::std::get<0>(ret) = s;
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
