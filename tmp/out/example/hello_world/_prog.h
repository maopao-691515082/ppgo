#pragma once

namespace ppgo
{

//mod namespaces:
//  mod6D07 __builtins
//  mod44C2 __builtins/strings
//  mod6EF5 __builtins/vecs
//  mod5247 example/hello_world

namespace mod6D07
{

::ppgo::Exc::Ptr func_throw(::std::tuple<> &ret, ::ppgo::Any::Ptr l_t);
::ppgo::Exc::Ptr func_println(::std::tuple<> &ret, ::ppgo::tp_string l_s);

}

namespace mod44C2
{

::ppgo::Exc::Ptr func_parse_int(::std::tuple<::ppgo::tp_int> &ret, ::ppgo::tp_string l_s);

}

namespace mod6EF5
{


}

namespace mod5247
{

::ppgo::Exc::Ptr func_main(::std::tuple<> &ret);

}

}
