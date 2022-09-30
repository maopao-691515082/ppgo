#pragma once
#include "mod6EF5-vecs.h"

namespace ppgo
{

//mod namespaces:
//  mod6D07 __builtins
//  mod44C2 __builtins/strings
//  mod6EF5 __builtins/vecs
//  modDD30 os
//  mod09BD test/bench/ppgostone
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

namespace mod09BD /* test/bench/ppgostone */
{

struct cls_Record;

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

namespace mod09BD /* test/bench/ppgostone */
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

namespace mod09BD /* test/bench/ppgostone */
{


struct cls_Record final : public virtual ::ppgo::Any
{
    
    virtual ::std::string R_TypeName() const override
    {
        return "test/bench/ppgostone.Record";
    }
    ::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> attr_ptr_comp;
    ::ppgo::tp_int attr_discr;
    ::ppgo::tp_int attr_enum_comp;
    ::ppgo::tp_int attr_int_comp;
    ::ppgo::tp_string attr_str_comp;
    virtual ::ppgo::Exc::Ptr method_assign(::std::tuple<> &ret, ::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> l_other) ;
    
    static std::string TypeName()
    {
        return "test/bench/ppgostone.Record";
    }
}
;

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

namespace mod09BD /* test/bench/ppgostone */
{

::ppgo::Exc::Ptr func_init(::std::tuple<> &ret);
::ppgo::Exc::Ptr func_func3(::std::tuple<::ppgo::tp_bool> &ret, ::ppgo::tp_int l_enum_par_in);
::ppgo::Exc::Ptr func_func2(::std::tuple<::ppgo::tp_bool> &ret, ::ppgo::tp_string l_str_par_i1, ::ppgo::tp_string l_str_par_i2);
::ppgo::Exc::Ptr func_func1(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_byte l_char_par1, ::ppgo::tp_byte l_char_par2);
::ppgo::Exc::Ptr func_proc8(::std::tuple<> &ret, ::ppgo::Vec<::ppgo::tp_int > l_array1_par, ::ppgo::Vec<::ppgo::Vec<::ppgo::tp_int > > l_array2_par, ::ppgo::tp_int l_int_par_i1, ::ppgo::tp_int l_int_par_i2);
::ppgo::Exc::Ptr func_proc7(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int l_int_par_i1, ::ppgo::tp_int l_int_par_i2);
::ppgo::Exc::Ptr func_proc6(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int l_enum_par_in);
::ppgo::Exc::Ptr func_proc5(::std::tuple<> &ret);
::ppgo::Exc::Ptr func_proc4(::std::tuple<> &ret);
::ppgo::Exc::Ptr func_proc3(::std::tuple<::ppgo::RCPtr<::ppgo::mod09BD::cls_Record>> &ret, ::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> l_ptr_par_out);
::ppgo::Exc::Ptr func_proc2(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_int l_int_par_io);
::ppgo::Exc::Ptr func_proc1(::std::tuple<::ppgo::RCPtr<::ppgo::mod09BD::cls_Record>> &ret, ::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> l_ptr_par_in);
::ppgo::Exc::Ptr func_proc0(::std::tuple<> &ret);
::ppgo::Exc::Ptr func_main(::std::tuple<> &ret);
extern ::ppgo::tp_int gv_loops;
extern ::ppgo::tp_int gv_Ident1;
extern ::ppgo::tp_int gv_Ident2;
extern ::ppgo::tp_int gv_Ident3;
extern ::ppgo::tp_int gv_Ident4;
extern ::ppgo::tp_int gv_Ident5;
extern ::ppgo::tp_int gv_int_glob;
extern ::ppgo::tp_bool gv_bool_glob;
extern ::ppgo::tp_byte gv_char1_glob;
extern ::ppgo::tp_byte gv_char2_glob;
extern ::ppgo::Vec<::ppgo::tp_int > gv_array1_glob;
extern ::ppgo::Vec<::ppgo::Vec<::ppgo::tp_int > > gv_array2_glob;
extern ::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> gv_ptr_glb;
extern ::ppgo::RCPtr<::ppgo::mod09BD::cls_Record> gv_ptr_glb_next;

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

namespace mod09BD /* test/bench/ppgostone */
{

::ppgo::Exc::Ptr init();

}

namespace mod07CC /* time */
{

::ppgo::Exc::Ptr init();

}

}
