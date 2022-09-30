#include "ppgo.h"
#include "_str_literals.h"

namespace ppgo
{


namespace mod6D07 /* __builtins */
{


::ppgo::Exc::Ptr init()
{
    static bool inited = false;
    if (inited) { return nullptr; } else { inited = true; }
    return nullptr;
}

}

namespace mod44C2 /* __builtins/strings */
{


::ppgo::Exc::Ptr func_parse_int(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_string l_s)
{
    ::std::get<0>(ret) = (({
::std::tuple<::ppgo::tp_int> ret_1301;
auto exc_1302 = ::ppgo::mod44C2::func_parse_int_with_base(ret_1301
, (l_s), (0LL)
);
if (exc_1302) {
exc_1302->PushTB("/Users/maopao/git/ppgo/lib/__builtins/strings/strings.ppgo", 6, "__builtins/strings.parse_int");
return exc_1302;}
::std::get<0>(ret_1301);
}));
    return nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr func_parse_uint(::std::tuple<::ppgo::tp_uint> &ret, ::ppgo::tp_string l_s)
{
    ::std::get<0>(ret) = (({
::std::tuple<::ppgo::tp_uint> ret_1303;
auto exc_1304 = ::ppgo::mod44C2::func_parse_uint_with_base(ret_1303
, (l_s), (0LL)
);
if (exc_1304) {
exc_1304->PushTB("/Users/maopao/git/ppgo/lib/__builtins/strings/strings.ppgo", 10, "__builtins/strings.parse_uint");
return exc_1304;}
::std::get<0>(ret_1303);
}));
    return nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr init()
{
    static bool inited = false;
    if (inited) { return nullptr; } else { inited = true; }
    return nullptr;
}

}

namespace mod6EF5 /* __builtins/vecs */
{


::ppgo::Exc::Ptr init()
{
    static bool inited = false;
    if (inited) { return nullptr; } else { inited = true; }
    return nullptr;
}

}

namespace modDD30 /* os */
{

::ppgo::Vec<::ppgo::tp_string > gv_args;

::ppgo::Exc::Ptr init()
{
    static bool inited = false;
    if (inited) { return nullptr; } else { inited = true; }
    return nullptr;
}

}

namespace mod09BD /* test/bench/ppgostone */
{


::ppgo::Exc::Ptr cls_Record::method_assign(::std::tuple<> &ret, ::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> l_other)
{
    ((this)->attr_ptr_comp) = ((l_other)->attr_ptr_comp);
    ((this)->attr_discr) = ((l_other)->attr_discr);
    ((this)->attr_enum_comp) = ((l_other)->attr_enum_comp);
    ((this)->attr_int_comp) = ((l_other)->attr_int_comp);
    ((this)->attr_str_comp) = ((l_other)->attr_str_comp);
    return nullptr;
}

::ppgo::Exc::Ptr func_init(::std::tuple<> &ret)
{
    (void)(({
::std::tuple<::ppgo::Vec<::ppgo::tp_int >> ret_1305;
auto exc_1306 = ::ppgo::mod6EF5::func_resize(ret_1305
, (::ppgo::mod09BD::gv_array1_glob), (51LL)
);
if (exc_1306) {
exc_1306->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 45, "test/bench/ppgostone.init");
return exc_1306;}
::std::get<0>(ret_1305);
}));
    
    {
        auto vec_1307 = (({
::std::tuple<::ppgo::Vec<::ppgo::Vec<::ppgo::tp_int > >> ret_1308;
auto exc_1309 = ::ppgo::mod6EF5::func_resize(ret_1308
, (::ppgo::mod09BD::gv_array2_glob), (51LL)
);
if (exc_1309) {
exc_1309->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 46, "test/bench/ppgostone.init");
return exc_1309;}
::std::get<0>(ret_1308);
}));
        
        for (::ppgo::tp_int vec_idx_1310 = 0; vec_idx_1310 < vec_1307.Len(); ++ vec_idx_1310)
        {
            ::ppgo::Vec<::ppgo::tp_int > l_v = vec_1307.Get(vec_idx_1310);
            (void)(({
::std::tuple<::ppgo::Vec<::ppgo::tp_int >> ret_1311;
auto exc_1312 = ::ppgo::mod6EF5::func_resize(ret_1311
, (l_v), (51LL)
);
if (exc_1312) {
exc_1312->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 48, "test/bench/ppgostone.init");
return exc_1312;}
::std::get<0>(ret_1311);
}));
        }
    }
    return nullptr;
}

::ppgo::Exc::Ptr func_func3(::std::tuple<::ppgo::tp_bool> &ret, ::ppgo::tp_int l_enum_par_in)
{
    ::ppgo::tp_int l_enum_loc = 0;
    l_enum_loc = (l_enum_par_in);
    ::std::get<0>(ret) = ((l_enum_loc) == (::ppgo::mod09BD::gv_Ident3));
    return nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr func_func2(::std::tuple<::ppgo::tp_bool> &ret, ::ppgo::tp_string l_str_par_i1, ::ppgo::tp_string l_str_par_i2)
{
    ::ppgo::tp_int l_int_loc = 0;
    l_int_loc = (1LL);
    ::ppgo::tp_byte l_char_loc = 0;
    
    while ((l_int_loc) <= (1LL))
    {
        
        if ((({
::std::tuple<::ppgo::tp_int> ret_1313;
auto exc_1314 = ::ppgo::mod09BD::func_func1(ret_1313
, ((l_str_par_i1).ByteAt(l_int_loc)), ((l_str_par_i2).ByteAt((l_int_loc) + (1LL)))
);
if (exc_1314) {
exc_1314->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 66, "test/bench/ppgostone.func2");
return exc_1314;}
::std::get<0>(ret_1313);
})) == (::ppgo::mod09BD::gv_Ident1))
        {
            (l_char_loc) = (65);
            ++ (l_int_loc);
        }
    }
    
    if (((l_char_loc) >= (87)) && ((l_char_loc) <= (90)))
    {
        (l_int_loc) = (7LL);
    }
    
    if ((l_char_loc) == (88))
    {
        ::std::get<0>(ret) = (true);
        return nullptr;
    }
    
    if ((l_str_par_i1).Cmp(l_str_par_i2) > 0)
    {
        (l_int_loc) += (7LL);
        ::std::get<0>(ret) = (true);
        return nullptr;
    }
    ::std::get<0>(ret) = (false);
    return nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr func_func1(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_byte l_char_par1, ::ppgo::tp_byte l_char_par2)
{
    ::ppgo::tp_byte l_char_loc1 = 0;
    l_char_loc1 = (l_char_par1);
    ::ppgo::tp_byte l_char_loc2 = 0;
    l_char_loc2 = (l_char_loc1);
    ::std::get<0>(ret) = (((l_char_loc2) != (l_char_par2)) ? (::ppgo::mod09BD::gv_Ident1) : (::ppgo::mod09BD::gv_Ident2));
    return nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr func_proc8(::std::tuple<> &ret, ::ppgo::Vec<::ppgo::tp_int > l_array1_par, ::ppgo::Vec<::ppgo::Vec<::ppgo::tp_int > > l_array2_par, ::ppgo::tp_int l_int_par_i1, ::ppgo::tp_int l_int_par_i2)
{
    ::ppgo::tp_int l_int_loc = 0;
    l_int_loc = ((l_int_par_i1) + (5LL));
    ((l_array1_par).GetForSet(l_int_loc)) = (l_int_par_i2);
    ((l_array1_par).GetForSet((l_int_loc) + (1LL))) = ((l_array1_par).Get(l_int_loc));
    ((l_array1_par).GetForSet((l_int_loc) + (30LL))) = (l_int_loc);
    
    for (::ppgo::tp_int idx_1315 = (l_int_loc), end_1316 = ((l_int_loc) + (2LL)); idx_1315 < end_1316; ++ idx_1315)
    {
        auto l_int_index = idx_1315;
        (((l_array2_par).Get(l_int_loc)).GetForSet(l_int_index)) = (l_int_loc);
    }
    ++ (((l_array2_par).Get(l_int_loc)).GetRef((l_int_loc) - (1LL)));
    (((l_array2_par).Get((l_int_loc) + (20LL))).GetForSet(l_int_loc)) = ((l_array1_par).Get(l_int_loc));
    (::ppgo::mod09BD::gv_int_glob) = (5LL);
    return nullptr;
}

::ppgo::Exc::Ptr func_proc7(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int l_int_par_i1, ::ppgo::tp_int l_int_par_i2)
{
    ::ppgo::tp_int l_int_loc = 0;
    l_int_loc = ((l_int_par_i1) + (2LL));
    ::ppgo::tp_int l_int_par_out = 0;
    l_int_par_out = ((l_int_par_i2) + (l_int_loc));
    ::std::get<0>(ret) = (l_int_par_out);
    return nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr func_proc6(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int l_enum_par_in)
{
    ::ppgo::tp_int l_enum_par_out = 0;
    l_enum_par_out = (l_enum_par_in);
    
    if (!(({
::std::tuple<::ppgo::tp_bool> ret_1317;
auto exc_1318 = ::ppgo::mod09BD::func_func3(ret_1317
, (l_enum_par_in)
);
if (exc_1318) {
exc_1318->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 124, "test/bench/ppgostone.proc6");
return exc_1318;}
::std::get<0>(ret_1317);
})))
    {
        (l_enum_par_out) = (::ppgo::mod09BD::gv_Ident4);
    }
    
    if ((l_enum_par_in) != (::ppgo::mod09BD::gv_Ident1))
    {
        (l_enum_par_out) = (::ppgo::mod09BD::gv_Ident1);
    }
    
    else if ((l_enum_par_in) == (::ppgo::mod09BD::gv_Ident2))
    {
        (l_enum_par_out) = (((::ppgo::mod09BD::gv_int_glob) > (100LL)) ? (::ppgo::mod09BD::gv_Ident1) : (::ppgo::mod09BD::gv_Ident4));
    }
    
    else if ((l_enum_par_in) == (::ppgo::mod09BD::gv_Ident3))
    {
        (l_enum_par_out) = (::ppgo::mod09BD::gv_Ident2);
    }
    
    else if ((l_enum_par_in) == (::ppgo::mod09BD::gv_Ident4))
    {
    }
    
    else if ((l_enum_par_in) == (::ppgo::mod09BD::gv_Ident5))
    {
        (l_enum_par_out) = (::ppgo::mod09BD::gv_Ident3);
    }
    ::std::get<0>(ret) = (l_enum_par_out);
    return nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr func_proc5(::std::tuple<> &ret)
{
    (::ppgo::mod09BD::gv_char1_glob) = (65);
    (::ppgo::mod09BD::gv_bool_glob) = (false);
    return nullptr;
}

::ppgo::Exc::Ptr func_proc4(::std::tuple<> &ret)
{
    ::ppgo::tp_bool l_bool_loc = false;
    l_bool_loc = ((::ppgo::mod09BD::gv_char1_glob) == (65));
    (l_bool_loc) = ((l_bool_loc) || (::ppgo::mod09BD::gv_bool_glob));
    (::ppgo::mod09BD::gv_char2_glob) = (66);
    return nullptr;
}

::ppgo::Exc::Ptr func_proc3(::std::tuple<::ppgo::RCPtr<::ppgo::mod09BD::cls_Record>> &ret, ::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> l_ptr_par_out)
{
    
    if ((::ppgo::mod09BD::gv_ptr_glb) != (::ppgo::RCPtr<::ppgo::mod09BD::cls_Record>(nullptr)))
    {
        (l_ptr_par_out) = ((::ppgo::mod09BD::gv_ptr_glb)->attr_ptr_comp);
    }
    
    else
    {
        (::ppgo::mod09BD::gv_int_glob) = (100LL);
    }
    ((::ppgo::mod09BD::gv_ptr_glb)->attr_int_comp) = (({
::std::tuple<::ppgo::tp_int> ret_1319;
auto exc_1320 = ::ppgo::mod09BD::func_proc7(ret_1319
, (10LL), (::ppgo::mod09BD::gv_int_glob)
);
if (exc_1320) {
exc_1320->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 173, "test/bench/ppgostone.proc3");
return exc_1320;}
::std::get<0>(ret_1319);
}));
    ::std::get<0>(ret) = (l_ptr_par_out);
    return nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr func_proc2(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int l_int_par_io)
{
    ::ppgo::tp_int l_int_loc = 0;
    l_int_loc = ((l_int_par_io) + (10LL));
    ::ppgo::tp_int l_enum_loc = 0;
    l_enum_loc = (0LL);
    
    while (true)
    {
        
        if ((::ppgo::mod09BD::gv_char1_glob) == (65))
        {
            -- (l_int_loc);
            (l_int_par_io) = ((l_int_loc) - (::ppgo::mod09BD::gv_int_glob));
            (l_enum_loc) = (::ppgo::mod09BD::gv_Ident1);
        }
        
        if ((l_enum_loc) == (::ppgo::mod09BD::gv_Ident1))
        {
            break;
        }
    }
    ::std::get<0>(ret) = (l_int_par_io);
    return nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr func_proc1(::std::tuple<::ppgo::RCPtr<::ppgo::mod09BD::cls_Record>> &ret, ::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> l_ptr_par_in)
{
    ::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> l_next_record;
    l_next_record = ((l_ptr_par_in)->attr_ptr_comp);
    (void)(({
::std::tuple<> ret_1321;
auto exc_1322 = (l_next_record)->method_assign(ret_1321
, (::ppgo::mod09BD::gv_ptr_glb)
);
if (exc_1322) {
exc_1322->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 202, "test/bench/ppgostone.proc1");
return exc_1322;}
ret_1321;
}));
    ((l_ptr_par_in)->attr_int_comp) = (5LL);
    ((l_next_record)->attr_int_comp) = ((l_ptr_par_in)->attr_int_comp);
    ((l_next_record)->attr_ptr_comp) = ((l_ptr_par_in)->attr_ptr_comp);
    ((l_next_record)->attr_ptr_comp) = (({
::std::tuple<::ppgo::RCPtr<::ppgo::mod09BD::cls_Record>> ret_1323;
auto exc_1324 = ::ppgo::mod09BD::func_proc3(ret_1323
, ((l_next_record)->attr_ptr_comp)
);
if (exc_1324) {
exc_1324->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 206, "test/bench/ppgostone.proc1");
return exc_1324;}
::std::get<0>(ret_1323);
}));
    
    if (((l_next_record)->attr_discr) == (::ppgo::mod09BD::gv_Ident1))
    {
        ((l_next_record)->attr_int_comp) = (6LL);
        ((l_next_record)->attr_enum_comp) = (({
::std::tuple<::ppgo::tp_int> ret_1325;
auto exc_1326 = ::ppgo::mod09BD::func_proc6(ret_1325
, ((l_ptr_par_in)->attr_enum_comp)
);
if (exc_1326) {
exc_1326->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 210, "test/bench/ppgostone.proc1");
return exc_1326;}
::std::get<0>(ret_1325);
}));
        ((l_next_record)->attr_ptr_comp) = ((::ppgo::mod09BD::gv_ptr_glb)->attr_ptr_comp);
        ((l_next_record)->attr_int_comp) = (({
::std::tuple<::ppgo::tp_int> ret_1327;
auto exc_1328 = ::ppgo::mod09BD::func_proc7(ret_1327
, ((l_next_record)->attr_int_comp), (10LL)
);
if (exc_1328) {
exc_1328->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 212, "test/bench/ppgostone.proc1");
return exc_1328;}
::std::get<0>(ret_1327);
}));
    }
    
    else
    {
        (void)(({
::std::tuple<> ret_1329;
auto exc_1330 = (l_ptr_par_in)->method_assign(ret_1329
, (l_next_record)
);
if (exc_1330) {
exc_1330->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 216, "test/bench/ppgostone.proc1");
return exc_1330;}
ret_1329;
}));
    }
    ::std::get<0>(ret) = (l_ptr_par_in);
    return nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr func_proc0(::std::tuple<> &ret)
{
    (::ppgo::mod09BD::gv_ptr_glb_next) = (({
::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> new_1331 = new ::ppgo::mod09BD::cls_Record;
new_1331;}));
    (::ppgo::mod09BD::gv_ptr_glb) = (({
::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> new_1332 = new ::ppgo::mod09BD::cls_Record;
new_1332;}));
    ((::ppgo::mod09BD::gv_ptr_glb)->attr_ptr_comp) = (::ppgo::mod09BD::gv_ptr_glb_next);
    ((::ppgo::mod09BD::gv_ptr_glb)->attr_discr) = (::ppgo::mod09BD::gv_Ident1);
    ((::ppgo::mod09BD::gv_ptr_glb)->attr_enum_comp) = (::ppgo::mod09BD::gv_Ident3);
    ((::ppgo::mod09BD::gv_ptr_glb)->attr_int_comp) = (40LL);
    ((::ppgo::mod09BD::gv_ptr_glb)->attr_str_comp) = (::ppgo::str_literals::s991);
    ::ppgo::tp_string l_string1_loc;
    l_string1_loc = (::ppgo::str_literals::s996);
    (((::ppgo::mod09BD::gv_array2_glob).Get(8LL)).GetForSet(7LL)) = (10LL);
    
    for (::ppgo::tp_int idx_1333 = (0LL), end_1334 = (::ppgo::mod09BD::gv_loops); idx_1333 < end_1334; ++ idx_1333)
    {
        auto l_i = idx_1333;
        (void)(({
::std::tuple<> ret_1335;
auto exc_1336 = ::ppgo::mod09BD::func_proc5(ret_1335
);
if (exc_1336) {
exc_1336->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 238, "test/bench/ppgostone.proc0");
return exc_1336;}
ret_1335;
}));
        (void)(({
::std::tuple<> ret_1337;
auto exc_1338 = ::ppgo::mod09BD::func_proc4(ret_1337
);
if (exc_1338) {
exc_1338->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 239, "test/bench/ppgostone.proc0");
return exc_1338;}
ret_1337;
}));
        ::ppgo::tp_int l_int_loc1 = 0;
        l_int_loc1 = (2LL);
        ::ppgo::tp_int l_int_loc2 = 0;
        l_int_loc2 = (3LL);
        ::ppgo::tp_int l_enum_loc = 0;
        l_enum_loc = (::ppgo::mod09BD::gv_Ident2);
        ::ppgo::tp_string l_string2_loc;
        l_string2_loc = (::ppgo::str_literals::s1042);
        (::ppgo::mod09BD::gv_bool_glob) = (!(({
::std::tuple<::ppgo::tp_bool> ret_1339;
auto exc_1340 = ::ppgo::mod09BD::func_func2(ret_1339
, (l_string1_loc), (l_string2_loc)
);
if (exc_1340) {
exc_1340->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 246, "test/bench/ppgostone.proc0");
return exc_1340;}
::std::get<0>(ret_1339);
})));
        ::ppgo::tp_int l_int_loc3 = 0;
        l_int_loc3 = (0LL);
        
        while ((l_int_loc1) < (l_int_loc2))
        {
            (l_int_loc3) = (((5LL) * (l_int_loc1)) - (l_int_loc2));
            (l_int_loc3) = (({
::std::tuple<::ppgo::tp_int> ret_1341;
auto exc_1342 = ::ppgo::mod09BD::func_proc7(ret_1341
, (l_int_loc1), (l_int_loc2)
);
if (exc_1342) {
exc_1342->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 251, "test/bench/ppgostone.proc0");
return exc_1342;}
::std::get<0>(ret_1341);
}));
            ++ (l_int_loc1);
        }
        (void)(({
::std::tuple<> ret_1343;
auto exc_1344 = ::ppgo::mod09BD::func_proc8(ret_1343
, (::ppgo::mod09BD::gv_array1_glob), (::ppgo::mod09BD::gv_array2_glob), (l_int_loc1), (l_int_loc3)
);
if (exc_1344) {
exc_1344->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 254, "test/bench/ppgostone.proc0");
return exc_1344;}
ret_1343;
}));
        (::ppgo::mod09BD::gv_ptr_glb) = (({
::std::tuple<::ppgo::RCPtr<::ppgo::mod09BD::cls_Record>> ret_1345;
auto exc_1346 = ::ppgo::mod09BD::func_proc1(ret_1345
, (::ppgo::mod09BD::gv_ptr_glb)
);
if (exc_1346) {
exc_1346->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 255, "test/bench/ppgostone.proc0");
return exc_1346;}
::std::get<0>(ret_1345);
}));
        
        for (::ppgo::tp_byte idx_1347 = (65), end_1348 = ((::ppgo::mod09BD::gv_char2_glob) + (2)); idx_1347 < end_1348; ++ idx_1347)
        {
            auto l_char_idx = idx_1347;
            
            if ((l_enum_loc) == (({
::std::tuple<::ppgo::tp_int> ret_1349;
auto exc_1350 = ::ppgo::mod09BD::func_func1(ret_1349
, (l_char_idx), (67)
);
if (exc_1350) {
exc_1350->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 258, "test/bench/ppgostone.proc0");
return exc_1350;}
::std::get<0>(ret_1349);
})))
            {
                (l_enum_loc) = (({
::std::tuple<::ppgo::tp_int> ret_1351;
auto exc_1352 = ::ppgo::mod09BD::func_proc6(ret_1351
, (::ppgo::mod09BD::gv_Ident1)
);
if (exc_1352) {
exc_1352->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 260, "test/bench/ppgostone.proc0");
return exc_1352;}
::std::get<0>(ret_1351);
}));
            }
        }
        (l_int_loc3) = ((l_int_loc2) * (l_int_loc1));
        (l_int_loc2) = ((l_int_loc3) / (l_int_loc1));
        (l_int_loc2) = (((7LL) * ((l_int_loc3) - (l_int_loc2))) - (l_int_loc1));
        (l_int_loc1) = (({
::std::tuple<::ppgo::tp_int> ret_1353;
auto exc_1354 = ::ppgo::mod09BD::func_proc2(ret_1353
, (l_int_loc1)
);
if (exc_1354) {
exc_1354->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 266, "test/bench/ppgostone.proc0");
return exc_1354;}
::std::get<0>(ret_1353);
}));
    }
    return nullptr;
}

::ppgo::Exc::Ptr func_main(::std::tuple<> &ret)
{
    
    if ((({
::std::tuple<::ppgo::tp_int> ret_1355;
auto exc_1356 = ::ppgo::mod6EF5::func_len(ret_1355
, (::ppgo::modDD30::gv_args)
);
if (exc_1356) {
exc_1356->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 272, "test/bench/ppgostone.main");
return exc_1356;}
::std::get<0>(ret_1355);
})) > (1LL))
    {
        ::ppgo::Any::Ptr l_exc;
        ::ppgo::tp_int ret_1360 = 0;
        auto exc_1361 = ([&] (::ppgo::tp_int &ret_1359) -> ::ppgo::Exc::Ptr { ret_1359 = (({
::std::tuple<::ppgo::tp_int> ret_1357;
auto exc_1358 = ::ppgo::mod44C2::func_parse_int(ret_1357
, ((::ppgo::modDD30::gv_args).Get(1LL))
);
if (exc_1358) {
exc_1358->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 275, "test/bench/ppgostone.main");
return exc_1358;}
::std::get<0>(ret_1357);
})); return nullptr; })(ret_1360);
        if (exc_1361) {
        (l_exc) = exc_1361->Throwed();
        } else {
        (::ppgo::mod09BD::gv_loops) = ret_1360;
        (l_exc) = ::ppgo::Any::Ptr();
        }
        
        if (((l_exc) != (::ppgo::Any::Ptr(nullptr))) || ((::ppgo::mod09BD::gv_loops) <= (0LL)))
        {
            (void)(({
::std::tuple<> ret_1362;
auto exc_1363 = ::ppgo::mod6D07::func_println(ret_1362
, (::ppgo::str_literals::s1223)
);
if (exc_1363) {
exc_1363->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 278, "test/bench/ppgostone.main");
return exc_1363;}
ret_1362;
}));
            return nullptr;
        }
    }
    ::ppgo::tp_float l_ts = 0;
    l_ts = (({
::std::tuple<::ppgo::tp_float> ret_1364;
auto exc_1365 = ::ppgo::mod07CC::func_time(ret_1364
);
if (exc_1365) {
exc_1365->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 282, "test/bench/ppgostone.main");
return exc_1365;}
::std::get<0>(ret_1364);
}));
    (void)(({
::std::tuple<> ret_1366;
auto exc_1367 = ::ppgo::mod09BD::func_proc0(ret_1366
);
if (exc_1367) {
exc_1367->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 283, "test/bench/ppgostone.main");
return exc_1367;}
ret_1366;
}));
    ::ppgo::tp_float l_tm = 0;
    l_tm = ((({
::std::tuple<::ppgo::tp_float> ret_1368;
auto exc_1369 = ::ppgo::mod07CC::func_time(ret_1368
);
if (exc_1369) {
exc_1369->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 284, "test/bench/ppgostone.main");
return exc_1369;}
::std::get<0>(ret_1368);
})) - (l_ts));
    (void)(({
::std::tuple<> ret_1370;
auto exc_1371 = ::ppgo::mod6D07::func_println(ret_1370
, (::ppgo::tp_string::Sprintf("Time used: %f sec", l_tm))
);
if (exc_1371) {
exc_1371->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 285, "test/bench/ppgostone.main");
return exc_1371;}
ret_1370;
}));
    (void)(({
::std::tuple<> ret_1372;
auto exc_1373 = ::ppgo::mod6D07::func_println(ret_1372
, (::ppgo::tp_string::Sprintf("This machine benchmarks at %f PPGoStones/second", (::ppgo::tp_float(::ppgo::mod09BD::gv_loops)) / (l_tm)))
);
if (exc_1373) {
exc_1373->PushTB("/Users/maopao/git/ppgo/lib/test/bench/ppgostone/dhrystone.ppgo", 286, "test/bench/ppgostone.main");
return exc_1373;}
ret_1372;
}));
    return nullptr;
}
::ppgo::tp_int gv_loops;
::ppgo::tp_int gv_Ident1;
::ppgo::tp_int gv_Ident2;
::ppgo::tp_int gv_Ident3;
::ppgo::tp_int gv_Ident4;
::ppgo::tp_int gv_Ident5;
::ppgo::tp_int gv_int_glob;
::ppgo::tp_bool gv_bool_glob;
::ppgo::tp_byte gv_char1_glob;
::ppgo::tp_byte gv_char2_glob;
::ppgo::Vec<::ppgo::tp_int > gv_array1_glob;
::ppgo::Vec<::ppgo::Vec<::ppgo::tp_int > > gv_array2_glob;
::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> gv_ptr_glb;
::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> gv_ptr_glb_next;

::ppgo::Exc::Ptr init()
{
    static bool inited = false;
    if (inited) { return nullptr; } else { inited = true; }
    
    {
        auto exc = ::ppgo::modDD30::init();
        if (exc) { return exc; }
    }
    
    {
        auto exc = ::ppgo::mod07CC::init();
        if (exc) { return exc; }
    }
    gv_loops = (5000000LL);
    gv_Ident1 = (1LL);
    gv_Ident2 = (2LL);
    gv_Ident3 = (3LL);
    gv_Ident4 = (4LL);
    gv_Ident5 = (5LL);
    ::std::tuple<> ret; return func_init(ret);
}

}

namespace mod07CC /* time */
{


::ppgo::Exc::Ptr init()
{
    static bool inited = false;
    if (inited) { return nullptr; } else { inited = true; }
    return nullptr;
}

}

Exc::Ptr main()
{
    ::std::tuple<> ret;
    return ::ppgo::mod09BD::init() ?: ::ppgo::mod09BD::func_main(ret);
}

}
