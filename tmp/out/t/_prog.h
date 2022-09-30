#pragma once
#include "mod6EF5-vecs.h"

namespace ppgo
{

//mod namespaces:
//  mod6D07 __builtins
//  mod44C2 __builtins/strings
//  mod6EF5 __builtins/vecs
//  mod879D math/rand
//  modDD30 os
//  modE358 t
//  mod07CC time

namespace mod6D07 /* __builtins */
{

struct cls_TypeAssertionException;

}

namespace mod44C2 /* __builtins/strings */
{


}

namespace mod6EF5 /* __builtins/vecs */
{


}

namespace modDD30 /* os */
{


}

namespace modE358 /* t */
{


}

namespace mod879D /* math/rand */
{


}

namespace mod07CC /* time */
{


}

namespace mod6D07 /* __builtins */
{


}

namespace mod44C2 /* __builtins/strings */
{


}

namespace mod6EF5 /* __builtins/vecs */
{


}

namespace modDD30 /* os */
{


}

namespace modE358 /* t */
{


}

namespace mod879D /* math/rand */
{


}

namespace mod07CC /* time */
{


}

namespace mod6D07 /* __builtins */
{


struct cls_TypeAssertionException final : public virtual ::ppgo::Any
{
    
    virtual ::std::string R_TypeName() const override
    {
        return "__builtins.TypeAssertionException";
    }
    
    static std::string TypeName()
    {
        return "__builtins.TypeAssertionException";
    }
}
;

}

namespace mod44C2 /* __builtins/strings */
{


}

namespace mod6EF5 /* __builtins/vecs */
{


}

namespace modDD30 /* os */
{


}

namespace modE358 /* t */
{


}

namespace mod879D /* math/rand */
{


}

namespace mod07CC /* time */
{


}

namespace mod6D07 /* __builtins */
{

::ppgo::Exc::Ptr func_throw(::std::tuple<> &ret, ::ppgo::Any::Ptr l_t);
::ppgo::Exc::Ptr func_println(::std::tuple<> &ret, ::ppgo::tp_string l_s);
::ppgo::Exc::Ptr func_assert(::std::tuple<> &ret, ::ppgo::tp_bool l_cond);

}

namespace mod44C2 /* __builtins/strings */
{

::ppgo::Exc::Ptr func_parse_int_with_base(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_string l_s, ::ppgo::tp_int l_base);
::ppgo::Exc::Ptr func_parse_uint_with_base(::std::tuple<::ppgo::tp_uint> &ret, ::ppgo::tp_string l_s, ::ppgo::tp_int l_base);
::ppgo::Exc::Ptr func_parse_int(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_string l_s);
::ppgo::Exc::Ptr func_parse_uint(::std::tuple<::ppgo::tp_uint> &ret, ::ppgo::tp_string l_s);
::ppgo::Exc::Ptr func_parse_float(::std::tuple<::ppgo::tp_float> &ret, ::ppgo::tp_string l_s);

}

namespace mod6EF5 /* __builtins/vecs */
{


}

namespace modDD30 /* os */
{

extern ::ppgo::Vec<::ppgo::tp_string > gv_args;

}

namespace modE358 /* t */
{

::ppgo::Exc::Ptr func_rand_prepare(::std::tuple<> &ret, ::ppgo::Vec<::ppgo::tp_int > l_a);
::ppgo::Exc::Ptr func_insert_sort(::std::tuple<> &ret, ::ppgo::Vec<::ppgo::tp_int > l_a, ::ppgo::tp_int l_begin, ::ppgo::tp_int l_end);
::ppgo::Exc::Ptr func_qsort(::std::tuple<> &ret, ::ppgo::Vec<::ppgo::tp_int > l_a, ::ppgo::tp_int l_begin, ::ppgo::tp_int l_end);
::ppgo::Exc::Ptr func_main(::std::tuple<> &ret);

}

namespace mod879D /* math/rand */
{

::ppgo::Exc::Ptr func_rand(::std::tuple<::ppgo::tp_float> &ret);
::ppgo::Exc::Ptr func_rand_n(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int l_n);
::ppgo::Exc::Ptr func_fast_rand_n(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int l_n);

}

namespace mod07CC /* time */
{

::ppgo::Exc::Ptr func_time(::std::tuple<::ppgo::tp_float> &ret);

}

namespace mod6D07 /* __builtins */
{

::ppgo::Exc::Ptr init();

}

namespace mod44C2 /* __builtins/strings */
{

::ppgo::Exc::Ptr init();

}

namespace mod6EF5 /* __builtins/vecs */
{

::ppgo::Exc::Ptr init();

}

namespace modDD30 /* os */
{

::ppgo::Exc::Ptr init();

}

namespace modE358 /* t */
{

::ppgo::Exc::Ptr init();

}

namespace mod879D /* math/rand */
{

::ppgo::Exc::Ptr init();

}

namespace mod07CC /* time */
{

::ppgo::Exc::Ptr init();

}

}
