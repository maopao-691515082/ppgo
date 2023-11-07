#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_WriteBatch::method_size(::std::tuple<::ppgo::tp_int> &ret)
{
    std::get<0>(ret) = nas.wb.Size();
    return nullptr;
}
::ppgo::Exc::Ptr cls_WriteBatch::method_clear(::std::tuple<> &ret)
{
    nas.wb.Clear();
    return nullptr;
}
::ppgo::Exc::Ptr cls_WriteBatch::method_set(::std::tuple<> &ret, ::ppgo::tp_string k, ::ppgo::tp_string v)
{
    nas.wb.Set(k.RawStr(), v.RawStr());
    return nullptr;
}
::ppgo::Exc::Ptr cls_WriteBatch::method_del(::std::tuple<> &ret, ::ppgo::tp_string k)
{
    nas.wb.Del(k.RawStr());
    return nullptr;
}

::ppgo::Exc::Ptr cls_Snapshot::method_get(
    ::std::tuple<::ppgo::tp_string, ::ppgo::tp_bool> &ret, ::ppgo::tp_string k)
{
    Str v;
    auto err = nas.s->Get(k.RawStr(), v, std::get<1>(ret));
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    std::get<0>(ret) = v;
    return nullptr;
}
::ppgo::Exc::Ptr cls_Snapshot::method_new_iter(::std::tuple<::std::shared_ptr<cls_Iter>> &ret)
{
    auto iter = ::std::make_shared<cls_Iter>();
    iter->nas.iter = nas.s->NewIterator();
    std::get<0>(ret) = iter;
    return nullptr;
}
::ppgo::Exc::Ptr cls_Snapshot::method_set(::std::tuple<> &ret, ::ppgo::tp_string k, ::ppgo::tp_string v)
{
    nas.s->wb.Set(k.RawStr(), v.RawStr());
    return nullptr;
}
::ppgo::Exc::Ptr cls_Snapshot::method_del(::std::tuple<> &ret, ::ppgo::tp_string k)
{
    nas.s->wb.Del(k.RawStr());
    return nullptr;
}
::ppgo::Exc::Ptr cls_Snapshot::method_export_wb(::std::tuple<::std::shared_ptr<cls_WriteBatch>> &ret)
{
    auto wb = ::std::make_shared<cls_WriteBatch>();
    wb->nas.wb = nas.s->wb;
    std::get<0>(ret) = wb;
    return nullptr;
}

::ppgo::Exc::Ptr cls_Iter::method_is_left_border(::std::tuple<::ppgo::tp_bool> &ret)
{
    bool b = nas.iter->IsLeftBorder();
    if (nas.iter->Err())
    {
        return ::ppgo::Exc::FromLomErr(nas.iter->Err());
    }
    std::get<0>(ret) = b;
    return nullptr;
}
::ppgo::Exc::Ptr cls_Iter::method_is_right_border(::std::tuple<::ppgo::tp_bool> &ret)
{
    bool b = nas.iter->IsRightBorder();
    if (nas.iter->Err())
    {
        return ::ppgo::Exc::FromLomErr(nas.iter->Err());
    }
    std::get<0>(ret) = b;
    return nullptr;
}
::ppgo::Exc::Ptr cls_Iter::method_valid(::std::tuple<::ppgo::tp_bool> &ret)
{
    bool b = nas.iter->Valid();
    if (nas.iter->Err())
    {
        return ::ppgo::Exc::FromLomErr(nas.iter->Err());
    }
    std::get<0>(ret) = b;
    return nullptr;
}

::ppgo::Exc::Ptr cls_Iter::method_key(::std::tuple<::ppgo::tp_string> &ret)
{
    if (nas.iter->Err())
    {
        return ::ppgo::Exc::FromLomErr(nas.iter->Err());
    }
    if (!nas.iter->Valid())
    {
        return ::ppgo::Exc::Sprintf("invalid iter");
    }
    std::get<0>(ret) = nas.iter->Key();
    return nullptr;
}
::ppgo::Exc::Ptr cls_Iter::method_value(::std::tuple<::ppgo::tp_string> &ret)
{
    if (nas.iter->Err())
    {
        return ::ppgo::Exc::FromLomErr(nas.iter->Err());
    }
    if (!nas.iter->Valid())
    {
        return ::ppgo::Exc::Sprintf("invalid iter");
    }
    std::get<0>(ret) = nas.iter->Value();
    return nullptr;
}
::ppgo::Exc::Ptr cls_Iter::method_kv(::std::tuple<::ppgo::tp_string, ::ppgo::tp_string> &ret)
{
    if (nas.iter->Err())
    {
        return ::ppgo::Exc::FromLomErr(nas.iter->Err());
    }
    if (!nas.iter->Valid())
    {
        return ::ppgo::Exc::Sprintf("invalid iter");
    }
    std::get<0>(ret) = nas.iter->Key();
    std::get<1>(ret) = nas.iter->Value();
    return nullptr;
}
::ppgo::Exc::Ptr cls_Iter::method_seek_first(::std::tuple<> &ret)
{
    nas.iter->SeekFirst();
    return nas.iter->Err() ? ::ppgo::Exc::FromLomErr(nas.iter->Err()) : nullptr;
}
::ppgo::Exc::Ptr cls_Iter::method_seek_last(::std::tuple<> &ret)
{
    nas.iter->SeekLast();
    return nas.iter->Err() ? ::ppgo::Exc::FromLomErr(nas.iter->Err()) : nullptr;
}
::ppgo::Exc::Ptr cls_Iter::method_seek(::std::tuple<> &ret, ::ppgo::tp_string k)
{
    nas.iter->Seek(k);
    return nas.iter->Err() ? ::ppgo::Exc::FromLomErr(nas.iter->Err()) : nullptr;
}
::ppgo::Exc::Ptr cls_Iter::method_seek_prev(::std::tuple<> &ret, ::ppgo::tp_string k)
{
    nas.iter->SeekPrev(k);
    return nas.iter->Err() ? ::ppgo::Exc::FromLomErr(nas.iter->Err()) : nullptr;
}
::ppgo::Exc::Ptr cls_Iter::method_move(
    ::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int step, ::std::optional<::ppgo::tp_string> stop_at)
{
    auto moved_step = nas.iter->Move(step, stop_at);
    if (nas.iter->Err())
    {
        return ::ppgo::Exc::FromLomErr(nas.iter->Err());
    }
    Assert(moved_step >= 0);
    std::get<0>(ret) = moved_step;
    return nullptr;
}

}

}
