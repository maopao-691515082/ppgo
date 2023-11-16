#pragma once

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

template <typename E>
::ppgo::Exc::Ptr func_len(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::Vec<E> v)
{
    ::std::get<0>(ret) = v.Len();
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_resize(::std::tuple<::ppgo::Vec<E>> &ret, ::ppgo::Vec<E> v, ::ppgo::tp_int sz)
{
    v.Resize(sz);
    ::std::get<0>(ret) = v;
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_get(::std::tuple<E> &ret, ::ppgo::Vec<E> v, ::ppgo::tp_int idx)
{
    if (idx < 0 || idx >= v.Len())
    {
        return ::ppgo::Exc::Sprintf("vector.get: index out of range");
    }
    ::std::get<0>(ret) = v.Get(idx);
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_set(
    ::std::tuple<::ppgo::Vec<E>> &ret, ::ppgo::Vec<E> v, ::ppgo::tp_int idx, E e)
{
    if (idx < 0 || idx >= v.Len())
    {
        return ::ppgo::Exc::Sprintf("vector.set: index out of range");
    }
    v.GetForSet(idx) = e;
    ::std::get<0>(ret) = v;
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_append(::std::tuple<::ppgo::Vec<E>> &ret, ::ppgo::Vec<E> v, E e)
{
    v.Append(e);
    ::std::get<0>(ret) = v;
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_extend(
    ::std::tuple<::ppgo::Vec<E>> &ret, ::ppgo::Vec<E> v, ::ppgo::VecView<E> es)
{
    if (!es.Valid())
    {
        return ::ppgo::Exc::Sprintf("vector.insert: invalid view");
    }
    v.InsertVecView(v.Len(), es);
    ::std::get<0>(ret) = v;
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_insert(
    ::std::tuple<::ppgo::Vec<E>> &ret, ::ppgo::Vec<E> v, ::ppgo::tp_int idx, E e)
{
    if (idx < 0 || idx > v.Len())
    {
        return ::ppgo::Exc::Sprintf("vector.insert: index out of range");
    }
    v.Insert(idx, e);
    ::std::get<0>(ret) = v;
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_insert_all(
    ::std::tuple<::ppgo::Vec<E>> &ret, ::ppgo::Vec<E> v, ::ppgo::tp_int idx, ::ppgo::VecView<E> es)
{
    if (!es.Valid())
    {
        return ::ppgo::Exc::Sprintf("vector.insert: invalid view");
    }
    if (idx < 0 || idx > v.Len())
    {
        return ::ppgo::Exc::Sprintf("vector.insert_all: index out of range");
    }
    v.InsertVecView(idx, es);
    ::std::get<0>(ret) = v;
    return nullptr;
}

template <typename E>
::ppgo::Exc::Ptr func_pop(
    ::std::tuple<E> &ret, ::ppgo::Vec<E> v, std::optional<::ppgo::tp_int> opt_idx)
{
    ::ppgo::tp_int idx = opt_idx.value_or(v.Len() - 1);
    if (idx < 0 || idx >= v.Len())
    {
        return ::ppgo::Exc::Sprintf("vector.pop: index out of range");
    }
    ::std::get<0>(ret) = v.Pop(idx);
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
