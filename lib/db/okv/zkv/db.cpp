#include "ppgo.h"

#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

::ppgo::Exc::Ptr cls_DB::method_write(::std::tuple<> &ret, decltype(cls__native_type_helper().attr_wb) wb)
{
    auto err = nas.db->Write(wb->nas.wb);
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    return nullptr;
}

typedef decltype(cls__native_type_helper().attr_snapshot) cls_ptr_Snapshot;

::ppgo::Exc::Ptr cls_DB::method_new_snapshot(::std::tuple<cls_ptr_Snapshot> &ret)
{
    auto snapshot = std::make_shared<cls_ptr_Snapshot::element_type>();
    snapshot->nas.s = nas.db->NewSnapshot();
    std::get<0>(ret) = snapshot;
    return nullptr;
}

::ppgo::Exc::Ptr cls_DB::method_space_cost(::std::tuple<::ppgo::tp_int> &ret)
{
    std::get<0>(ret) = nas.db->SpaceCost();
    return nullptr;
}

::ppgo::Exc::Ptr func_open_m(::std::tuple<::std::shared_ptr<cls_DB>> &ret)
{
    auto db = ::std::make_shared<cls_DB>();
    auto err = ::lom::ordered_kv::zkv::DB::Open(db->nas.db);
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }
    std::get<0>(ret) = db;
    return nullptr;
}

::ppgo::Exc::Ptr func_open_p(
    ::std::tuple<::std::shared_ptr<cls_DB>> &ret,
    ::ppgo::tp_string path, ::ppgo::tp_bool create_if_missing,
    ::std::shared_ptr<intf_ErrMsgHandler> bg_err_msg_handler)
{
    ::lom::ordered_kv::zkv::DB::Options opts;
    opts.create_if_missing = create_if_missing;
    if (bg_err_msg_handler)
    {
        opts.handle_bg_err = [bg_err_msg_handler] (LOM_ERR err) {
            std::tuple<> r;
            bg_err_msg_handler->method_handle(r, err->Msg());
        };
    }

    ::lom::ordered_kv::zkv::DB::Ptr zkv_db;
    auto err = ::lom::ordered_kv::zkv::DB::Open(path.Data(), zkv_db, opts);
    if (err)
    {
        return ::ppgo::Exc::FromLomErr(err);
    }

    auto db = ::std::make_shared<cls_DB>();
    db->nas.db = zkv_db;
    std::get<0>(ret) = db;
    return nullptr;
}

}

}

#pragma ppgo undef-THIS_MOD
