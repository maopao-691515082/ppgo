#include "ppgo.h"

namespace ppgo
{

tp_string::tp_string(const Vec<char8_t> &v) :
    tp_string(reinterpret_cast<const char *>(&v.GetRef(0)), v.Len())
{
}

tp_string::tp_string(const VecView<char8_t> &vv) :
    tp_string(reinterpret_cast<const char *>(&vv.GetRef(0)), vv.Len())
{
}

}
