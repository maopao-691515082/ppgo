#include "ppgo.h"

namespace ppgo
{

Exc::Ptr (*main)(::std::tuple<> &) = &::ppgo::mod5247::func_main;

namespace mod6D07
{


}

namespace mod44C2
{


}

namespace mod6EF5
{


}

namespace mod5247
{


::ppgo::Exc::Ptr func_main(::std::tuple<> &ret)
{
    (void)(({
::std::tuple<> ret_58;
auto exc_59 = ::ppgo::mod6D07::func_println(ret_58
, (::ppgo::tp_string("hello world", 11))
);
if (exc_59) {
exc_59->PushTB("/Users/maopao/git/ppgo/lib/example/hello_world/hello_world.ppgo", 3, "example/hello_world.main");
return exc_59;}
ret_58;
}));
    return nullptr;
}

}

}
