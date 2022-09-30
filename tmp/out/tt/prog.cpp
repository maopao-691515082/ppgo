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
::std::tuple<::ppgo::tp_int> ret_277;
auto exc_278 = ::ppgo::mod44C2::func_parse_int_with_base(ret_277
, (l_s), (0LL)
);
if (exc_278) {
exc_278->PushTB("/Users/maopao/git/ppgo/lib/__builtins/strings/strings.ppgo", 6, "__builtins/strings.parse_int");
return exc_278;}
::std::get<0>(ret_277);
}));
    return nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr func_parse_uint(::std::tuple<::ppgo::tp_uint> &ret, ::ppgo::tp_string l_s)
{
    ::std::get<0>(ret) = (({
::std::tuple<::ppgo::tp_uint> ret_279;
auto exc_280 = ::ppgo::mod44C2::func_parse_uint_with_base(ret_279
, (l_s), (0LL)
);
if (exc_280) {
exc_280->PushTB("/Users/maopao/git/ppgo/lib/__builtins/strings/strings.ppgo", 10, "__builtins/strings.parse_uint");
return exc_280;}
::std::get<0>(ret_279);
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

namespace mod9374 /* __builtins/maps */
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

namespace modACCC /* tt */
{


::ppgo::Exc::Ptr func_main(::std::tuple<> &ret)
{
    ::ppgo::Map<::ppgo::tp_int, ::ppgo::tp_int > l_m;
    l_m = (::ppgo::Map<::ppgo::tp_int, ::ppgo::tp_int >({::std::pair<const ::ppgo::tp_int, ::ppgo::tp_int>((1LL), (2LL))}));
    ((l_m).GetForSet(2LL)) = (3LL);
    
    {
        auto map_281 = (l_m);
        
        for (auto map_iter_282 = map_281.NewIter(); map_iter_282.Valid(); map_iter_282.Inc())
        {
            ::ppgo::tp_int l_k = map_iter_282.Key();
            ::ppgo::tp_int l_v = map_iter_282.Value();
            (void)(({
::std::tuple<> ret_283;
auto exc_284 = ::ppgo::mod6D07::func_println(ret_283
, (::ppgo::tp_string::Sprintf("%s: %s", ::ppgo::Any::ToStr(::ppgo::base_type_boxing::Obj<::ppgo::tp_int>::New(l_k)).Data(), ::ppgo::Any::ToStr(::ppgo::base_type_boxing::Obj<::ppgo::tp_int>::New(l_v)).Data()))
);
if (exc_284) {
exc_284->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 7, "tt.main");
return exc_284;}
ret_283;
}));
        }
    }
    (void)(({
::std::tuple<> ret_285;
auto exc_286 = ::ppgo::mod6D07::func_println(ret_285
, (::ppgo::tp_string::Sprintf("%s", ::ppgo::Any::ToStr(::ppgo::base_type_boxing::Obj<::ppgo::tp_int>::New(({
::std::tuple<::ppgo::tp_int> ret_287;
auto exc_288 = ::ppgo::mod9374::func_len(ret_287
, (l_m)
);
if (exc_288) {
exc_288->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 10, "tt.main");
return exc_288;}
::std::get<0>(ret_287);
}))).Data()))
);
if (exc_286) {
exc_286->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 10, "tt.main");
return exc_286;}
ret_285;
}));
    (void)(({
::std::tuple<> ret_289;
auto exc_290 = ::ppgo::mod6D07::func_println(ret_289
, (::ppgo::tp_string::Sprintf("%s", ::ppgo::Any::ToStr(::ppgo::base_type_boxing::Obj<::ppgo::tp_bool>::New(({
::std::tuple<::ppgo::tp_bool> ret_291;
auto exc_292 = ::ppgo::mod9374::func_has_key(ret_291
, (l_m), (1LL)
);
if (exc_292) {
exc_292->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 11, "tt.main");
return exc_292;}
::std::get<0>(ret_291);
}))).Data()))
);
if (exc_290) {
exc_290->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 11, "tt.main");
return exc_290;}
ret_289;
}));
    (void)(({
::std::tuple<> ret_293;
auto exc_294 = ::ppgo::mod6D07::func_println(ret_293
, (::ppgo::tp_string::Sprintf("%s", ::ppgo::Any::ToStr(::ppgo::base_type_boxing::Obj<::ppgo::tp_int>::New(({
::std::tuple<::ppgo::tp_int> ret_295;
auto exc_296 = ::ppgo::mod9374::func_get(ret_295
, (l_m), (1LL)
);
if (exc_296) {
exc_296->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 12, "tt.main");
return exc_296;}
::std::get<0>(ret_295);
}))).Data()))
);
if (exc_294) {
exc_294->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 12, "tt.main");
return exc_294;}
ret_293;
}));
    (void)(({
::std::tuple<> ret_297;
auto exc_298 = ::ppgo::mod6D07::func_println(ret_297
, (::ppgo::tp_string::Sprintf("%s", ::ppgo::Any::ToStr(::ppgo::base_type_boxing::Obj<::ppgo::tp_int>::New(({
::std::tuple<::ppgo::tp_int> ret_299;
auto exc_300 = ::ppgo::mod9374::func_pop(ret_299
, (l_m), (1LL)
);
if (exc_300) {
exc_300->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 14, "tt.main");
return exc_300;}
::std::get<0>(ret_299);
}))).Data()))
);
if (exc_298) {
exc_298->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 14, "tt.main");
return exc_298;}
ret_297;
}));
    (void)(({
::std::tuple<> ret_301;
auto exc_302 = ::ppgo::mod6D07::func_println(ret_301
, (::ppgo::tp_string::Sprintf("%s", ::ppgo::Any::ToStr(::ppgo::base_type_boxing::Obj<::ppgo::tp_int>::New(({
::std::tuple<::ppgo::tp_int> ret_303;
auto exc_304 = ::ppgo::mod9374::func_len(ret_303
, (l_m)
);
if (exc_304) {
exc_304->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 16, "tt.main");
return exc_304;}
::std::get<0>(ret_303);
}))).Data()))
);
if (exc_302) {
exc_302->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 16, "tt.main");
return exc_302;}
ret_301;
}));
    (void)(({
::std::tuple<> ret_305;
auto exc_306 = ::ppgo::mod6D07::func_println(ret_305
, (::ppgo::tp_string::Sprintf("%s", ::ppgo::Any::ToStr(::ppgo::base_type_boxing::Obj<::ppgo::tp_bool>::New(({
::std::tuple<::ppgo::tp_bool> ret_307;
auto exc_308 = ::ppgo::mod9374::func_has_key(ret_307
, (l_m), (1LL)
);
if (exc_308) {
exc_308->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 17, "tt.main");
return exc_308;}
::std::get<0>(ret_307);
}))).Data()))
);
if (exc_306) {
exc_306->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 17, "tt.main");
return exc_306;}
ret_305;
}));
    (void)(({
::std::tuple<> ret_309;
auto exc_310 = ::ppgo::mod6D07::func_println(ret_309
, (::ppgo::tp_string::Sprintf("%s", ::ppgo::Any::ToStr(::ppgo::base_type_boxing::Obj<::ppgo::tp_int>::New(({
::std::tuple<::ppgo::tp_int> ret_311;
auto exc_312 = ::ppgo::mod9374::func_get(ret_311
, (l_m), (1LL)
);
if (exc_312) {
exc_312->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 18, "tt.main");
return exc_312;}
::std::get<0>(ret_311);
}))).Data()))
);
if (exc_310) {
exc_310->PushTB("/Users/maopao/git/ppgo/ulib/tt/tt.ppgo", 18, "tt.main");
return exc_310;}
ret_309;
}));
    return nullptr;
}

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
    return ::ppgo::modACCC::init() ?: ::ppgo::modACCC::func_main(ret);
}

}
