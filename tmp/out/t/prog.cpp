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
::std::tuple<::ppgo::tp_int> ret_838;
auto exc_839 = ::ppgo::mod44C2::func_parse_int_with_base(ret_838
, (l_s), (0LL)
);
if (exc_839) {
exc_839->PushTB("/Users/maopao/git/ppgo/lib/__builtins/strings/strings.ppgo", 6, "__builtins/strings.parse_int");
return exc_839;}
::std::get<0>(ret_838);
}));
    return nullptr;
    return nullptr;
}

::ppgo::Exc::Ptr func_parse_uint(::std::tuple<::ppgo::tp_uint> &ret, ::ppgo::tp_string l_s)
{
    ::std::get<0>(ret) = (({
::std::tuple<::ppgo::tp_uint> ret_840;
auto exc_841 = ::ppgo::mod44C2::func_parse_uint_with_base(ret_840
, (l_s), (0LL)
);
if (exc_841) {
exc_841->PushTB("/Users/maopao/git/ppgo/lib/__builtins/strings/strings.ppgo", 10, "__builtins/strings.parse_uint");
return exc_841;}
::std::get<0>(ret_840);
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

namespace modE358 /* t */
{


::ppgo::Exc::Ptr func_rand_prepare(::std::tuple<> &ret, ::ppgo::Vec<::ppgo::tp_int > l_a)
{
    ::ppgo::tp_int l_alen = 0;
    l_alen = (({
::std::tuple<::ppgo::tp_int> ret_842;
auto exc_843 = ::ppgo::mod6EF5::func_len(ret_842
, (l_a)
);
if (exc_843) {
exc_843->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 9, "t.rand_prepare");
return exc_843;}
::std::get<0>(ret_842);
}));
    ::ppgo::tp_int l_count = 0;
    l_count = (0LL);
    
    while ((l_count) < ((l_alen) / (1000LL)))
    {
        ::ppgo::tp_int l_i = 0;
        l_i = (({
::std::tuple<::ppgo::tp_int> ret_844;
auto exc_845 = ::ppgo::mod879D::func_fast_rand_n(ret_844
, (l_alen)
);
if (exc_845) {
exc_845->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 14, "t.rand_prepare");
return exc_845;}
::std::get<0>(ret_844);
}));
        ::ppgo::tp_int l_j = 0;
        l_j = (({
::std::tuple<::ppgo::tp_int> ret_846;
auto exc_847 = ::ppgo::mod879D::func_fast_rand_n(ret_846
, (l_alen)
);
if (exc_847) {
exc_847->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 15, "t.rand_prepare");
return exc_847;}
::std::get<0>(ret_846);
}));
        
        if ((((l_i) < (l_j)) && (((l_a).At(l_i)) > ((l_a).At(l_j)))) || (((l_i) > (l_j)) && (((l_a).At(l_i)) < ((l_a).At(l_j)))))
        {
            ::ppgo::tp_int l_tmp = 0;
            l_tmp = ((l_a).At(l_i));
            ((l_a).At(l_i)) = ((l_a).At(l_j));
            ((l_a).At(l_j)) = (l_tmp);
            (l_count) = (0LL);
        }
        
        else
        {
            ++ (l_count);
        }
    }
    return nullptr;
}

::ppgo::Exc::Ptr func_insert_sort(::std::tuple<> &ret, ::ppgo::Vec<::ppgo::tp_int > l_a, ::ppgo::tp_int l_begin, ::ppgo::tp_int l_end)
{
    (void)(({
::std::tuple<> ret_848;
auto exc_849 = ::ppgo::mod6D07::func_assert(ret_848
, ((l_begin) <= (l_end))
);
if (exc_849) {
exc_849->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 33, "t.insert_sort");
return exc_849;}
ret_848;
}));
    
    for (::ppgo::tp_int idx_850 = ((l_begin) + (1LL)), end_851 = (l_end); idx_850 < end_851; ++ idx_850)
    {
        auto l_i = idx_850;
        ::ppgo::tp_int l_n = 0;
        l_n = ((l_a).At(l_i));
        
        while (((l_i) > (l_begin)) && ((l_n) < ((l_a).At((l_i) - (1LL)))))
        {
            ((l_a).At(l_i)) = ((l_a).At((l_i) - (1LL)));
            -- (l_i);
        }
        ((l_a).At(l_i)) = (l_n);
    }
    return nullptr;
}

::ppgo::Exc::Ptr func_qsort(::std::tuple<> &ret, ::ppgo::Vec<::ppgo::tp_int > l_a, ::ppgo::tp_int l_begin, ::ppgo::tp_int l_end)
{
    (void)(({
::std::tuple<> ret_852;
auto exc_853 = ::ppgo::mod6D07::func_assert(ret_852
, ((l_begin) <= (l_end))
);
if (exc_853) {
exc_853->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 48, "t.qsort");
return exc_853;}
ret_852;
}));
    
    if (((l_end) - (l_begin)) < (10LL))
    {
        (void)(({
::std::tuple<> ret_854;
auto exc_855 = ::ppgo::modE358::func_insert_sort(ret_854
, (l_a), (l_begin), (l_end)
);
if (exc_855) {
exc_855->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 51, "t.qsort");
return exc_855;}
ret_854;
}));
        return nullptr;
    }
    ::ppgo::tp_int l_pivot = 0;
    l_pivot = ((l_a).At(l_begin));
    ::ppgo::tp_int l_i = 0;
    l_i = ((l_begin) + (1LL));
    ::ppgo::tp_int l_j = 0;
    l_j = ((l_end) - (1LL));
    
    while (true)
    {
        
        while (((l_i) <= (l_j)) && (((l_a).At(l_i)) <= (l_pivot)))
        {
            ++ (l_i);
        }
        
        while (((l_j) >= (l_i)) && (((l_a).At(l_j)) > (l_pivot)))
        {
            -- (l_j);
        }
        
        if ((l_i) > (l_j))
        {
            break;
        }
        (void)(({
::std::tuple<> ret_856;
auto exc_857 = ::ppgo::mod6D07::func_assert(ret_856
, ((l_i) != (l_j))
);
if (exc_857) {
exc_857->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 73, "t.qsort");
return exc_857;}
ret_856;
}));
        ::ppgo::tp_int l_tmp = 0;
        l_tmp = ((l_a).At(l_i));
        ((l_a).At(l_i)) = ((l_a).At(l_j));
        ((l_a).At(l_j)) = (l_tmp);
    }
    (void)(({
::std::tuple<> ret_858;
auto exc_859 = ::ppgo::mod6D07::func_assert(ret_858
, ((l_i) == ((l_j) + (1LL)))
);
if (exc_859) {
exc_859->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 78, "t.qsort");
return exc_859;}
ret_858;
}));
    ::ppgo::tp_int l_tmp = 0;
    l_tmp = ((l_a).At(l_j));
    ((l_a).At(l_j)) = ((l_a).At(l_begin));
    ((l_a).At(l_begin)) = (l_tmp);
    (void)(({
::std::tuple<> ret_860;
auto exc_861 = ::ppgo::modE358::func_qsort(ret_860
, (l_a), (l_begin), (l_j)
);
if (exc_861) {
exc_861->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 82, "t.qsort");
return exc_861;}
ret_860;
}));
    (void)(({
::std::tuple<> ret_862;
auto exc_863 = ::ppgo::modE358::func_qsort(ret_862
, (l_a), (l_i), (l_end)
);
if (exc_863) {
exc_863->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 83, "t.qsort");
return exc_863;}
ret_862;
}));
    return nullptr;
}

::ppgo::Exc::Ptr func_main(::std::tuple<> &ret)
{
    ::ppgo::tp_bool l_enable_rand_prepare = false;
    l_enable_rand_prepare = (((({
::std::tuple<::ppgo::tp_int> ret_864;
auto exc_865 = ::ppgo::mod6EF5::func_len(ret_864
, (::ppgo::modDD30::gv_args)
);
if (exc_865) {
exc_865->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 89, "t.main");
return exc_865;}
::std::get<0>(ret_864);
})) >= (3LL)) && (((::ppgo::modDD30::gv_args).At(2LL)).Cmp(::ppgo::str_literals::s569) == 0));
    ::ppgo::tp_bool l_use_qsort = false;
    l_use_qsort = (((({
::std::tuple<::ppgo::tp_int> ret_866;
auto exc_867 = ::ppgo::mod6EF5::func_len(ret_866
, (::ppgo::modDD30::gv_args)
);
if (exc_867) {
exc_867->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 90, "t.main");
return exc_867;}
::std::get<0>(ret_866);
})) >= (3LL)) && (((::ppgo::modDD30::gv_args).At(2LL)).Cmp(::ppgo::str_literals::s590) == 0));
    ::ppgo::tp_int l_data_count = 0;
    l_data_count = (({
::std::tuple<::ppgo::tp_int> ret_868;
auto exc_869 = ::ppgo::mod44C2::func_parse_int(ret_868
, (({
::std::tuple<::ppgo::tp_string> ret_870;
auto exc_871 = ::ppgo::mod6EF5::func_get(ret_870
, (::ppgo::modDD30::gv_args), (1LL)
);
if (exc_871) {
exc_871->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 91, "t.main");
return exc_871;}
::std::get<0>(ret_870);
}))
);
if (exc_869) {
exc_869->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 91, "t.main");
return exc_869;}
::std::get<0>(ret_868);
}));
    (void)(({
::std::tuple<> ret_872;
auto exc_873 = ::ppgo::mod6D07::func_println(ret_872
, (::ppgo::str_literals::s610)
);
if (exc_873) {
exc_873->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 94, "t.main");
return exc_873;}
ret_872;
}));
    ::ppgo::Vec<::ppgo::tp_int > l_a;
    l_a = (({
::std::tuple<::ppgo::Vec<::ppgo::tp_int >> ret_874;
auto exc_875 = ::ppgo::mod6EF5::func_resize(ret_874
, (::ppgo::Vec<::ppgo::tp_int >({})), (l_data_count)
);
if (exc_875) {
exc_875->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 96, "t.main");
return exc_875;}
::std::get<0>(ret_874);
}));
    
    for (::ppgo::tp_int idx_876 = (0LL), end_877 = (({
::std::tuple<::ppgo::tp_int> ret_878;
auto exc_879 = ::ppgo::mod6EF5::func_len(ret_878
, (l_a)
);
if (exc_879) {
exc_879->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 97, "t.main");
return exc_879;}
::std::get<0>(ret_878);
})); idx_876 < end_877; ++ idx_876)
    {
        auto l_i = idx_876;
        ((l_a).At(l_i)) = (({
::std::tuple<::ppgo::tp_int> ret_880;
auto exc_881 = ::ppgo::mod879D::func_fast_rand_n(ret_880
, (100000000LL)
);
if (exc_881) {
exc_881->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 99, "t.main");
return exc_881;}
::std::get<0>(ret_880);
}));
    }
    ::ppgo::tp_float l_ts = 0;
    l_ts = (({
::std::tuple<::ppgo::tp_float> ret_882;
auto exc_883 = ::ppgo::mod07CC::func_time(ret_882
);
if (exc_883) {
exc_883->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 102, "t.main");
return exc_883;}
::std::get<0>(ret_882);
}));
    
    if (l_use_qsort)
    {
        (void)(({
::std::tuple<> ret_884;
auto exc_885 = ::ppgo::modE358::func_qsort(ret_884
, (l_a), (0LL), (({
::std::tuple<::ppgo::tp_int> ret_886;
auto exc_887 = ::ppgo::mod6EF5::func_len(ret_886
, (l_a)
);
if (exc_887) {
exc_887->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 106, "t.main");
return exc_887;}
::std::get<0>(ret_886);
}))
);
if (exc_885) {
exc_885->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 106, "t.main");
return exc_885;}
ret_884;
}));
    }
    
    else
    {
        
        if (l_enable_rand_prepare)
        {
            (void)(({
::std::tuple<> ret_888;
auto exc_889 = ::ppgo::modE358::func_rand_prepare(ret_888
, (l_a)
);
if (exc_889) {
exc_889->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 112, "t.main");
return exc_889;}
ret_888;
}));
        }
        (void)(({
::std::tuple<> ret_890;
auto exc_891 = ::ppgo::modE358::func_insert_sort(ret_890
, (l_a), (0LL), (({
::std::tuple<::ppgo::tp_int> ret_892;
auto exc_893 = ::ppgo::mod6EF5::func_len(ret_892
, (l_a)
);
if (exc_893) {
exc_893->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 114, "t.main");
return exc_893;}
::std::get<0>(ret_892);
}))
);
if (exc_891) {
exc_891->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 114, "t.main");
return exc_891;}
ret_890;
}));
    }
    (void)(({
::std::tuple<> ret_894;
auto exc_895 = ::ppgo::mod6D07::func_println(ret_894
, (::ppgo::tp_string::Sprintf("time cost: %f sec", (({
::std::tuple<::ppgo::tp_float> ret_896;
auto exc_897 = ::ppgo::mod07CC::func_time(ret_896
);
if (exc_897) {
exc_897->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 117, "t.main");
return exc_897;}
::std::get<0>(ret_896);
})) - (l_ts)))
);
if (exc_895) {
exc_895->PushTB("/Users/maopao/git/ppgo/ulib/t/t.ppgo", 117, "t.main");
return exc_895;}
ret_894;
}));
    return nullptr;
}

::ppgo::Exc::Ptr init()
{
    static bool inited = false;
    if (inited) { return nullptr; } else { inited = true; }
    
    {
        auto exc = ::ppgo::modDD30::init();
        if (exc) { return exc; }
    }
    
    {
        auto exc = ::ppgo::mod879D::init();
        if (exc) { return exc; }
    }
    
    {
        auto exc = ::ppgo::mod07CC::init();
        if (exc) { return exc; }
    }
    return nullptr;
}

}

namespace mod879D /* math/rand */
{


::ppgo::Exc::Ptr func_rand_n(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int l_n)
{
    (void)(({
::std::tuple<> ret_898;
auto exc_899 = ::ppgo::mod6D07::func_assert(ret_898
, ((l_n) > (0LL))
);
if (exc_899) {
exc_899->PushTB("/Users/maopao/git/ppgo/lib/math/rand/rand.ppgo", 5, "math/rand.rand_n");
return exc_899;}
ret_898;
}));
    ::ppgo::tp_int l_r = 0;
    l_r = (((::ppgo::tp_int((({
::std::tuple<::ppgo::tp_float> ret_900;
auto exc_901 = ::ppgo::mod879D::func_rand(ret_900
);
if (exc_901) {
exc_901->PushTB("/Users/maopao/git/ppgo/lib/math/rand/rand.ppgo", 6, "math/rand.rand_n");
return exc_901;}
::std::get<0>(ret_900);
})) * (::ppgo::tp_float((1LL) << (31LL))))) << (32LL)) | (::ppgo::tp_int((({
::std::tuple<::ppgo::tp_float> ret_902;
auto exc_903 = ::ppgo::mod879D::func_rand(ret_902
);
if (exc_903) {
exc_903->PushTB("/Users/maopao/git/ppgo/lib/math/rand/rand.ppgo", 6, "math/rand.rand_n");
return exc_903;}
::std::get<0>(ret_902);
})) * (::ppgo::tp_float((1LL) << (32LL))))));
    (void)(({
::std::tuple<> ret_904;
auto exc_905 = ::ppgo::mod6D07::func_assert(ret_904
, ((l_r) >= (0LL))
);
if (exc_905) {
exc_905->PushTB("/Users/maopao/git/ppgo/lib/math/rand/rand.ppgo", 7, "math/rand.rand_n");
return exc_905;}
ret_904;
}));
    ::std::get<0>(ret) = ((l_r) % (l_n));
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
    return ::ppgo::modE358::init() ?: ::ppgo::modE358::func_main(ret);
}

}
