#pragma once

#ifdef PPGO_THIS_MOD
#error macro PPGO_THIS_MOD redefined
#endif
#define PPGO_THIS_MOD mod9374

namespace ppgo
{

namespace PPGO_THIS_MOD
{

template <typename K, typename V>
::ppgo::Exc::Ptr func_len(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::Map<K, V> m)
{
    ::std::get<0>(ret) = m.Len();
    return nullptr;
}

template <typename K, typename V>
::ppgo::Exc::Ptr func_has_key(::std::tuple<::ppgo::tp_bool> &ret, ::ppgo::Map<K, V> m, K k)
{
    ::std::get<0>(ret) = m.GetOrPop(k);
    return nullptr;
}

template <typename K, typename V>
::ppgo::Exc::Ptr func_get(::std::tuple<V> &ret, ::ppgo::Map<K, V> m, K k)
{
    if (m.GetOrPop(k, &::std::get<0>(ret)))
    {
        return nullptr;
    }
    return ::ppgo::Exc::New(::ppgo::base_type_boxing::StrObj::New(
        ::ppgo::tp_string("map.get: no such key")));
}

template <typename K, typename V>
::ppgo::Exc::Ptr func_pop(::std::tuple<V> &ret, ::ppgo::Map<K, V> m, K k)
{
    if (m.GetOrPop(k, &::std::get<0>(ret), true))
    {
        return nullptr;
    }
    return ::ppgo::Exc::New(::ppgo::base_type_boxing::StrObj::New(
        ::ppgo::tp_string("map.pop: no such key")));
}

}

}

#undef PPGO_THIS_MOD
