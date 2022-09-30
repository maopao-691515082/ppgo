#pragma once

#include "_internal.h"
#include "_mem.h"
#include "_base_type.h"
#include "_exc.h"

namespace ppgo
{

extern Exc::Ptr (*main)(std::tuple<> &);

}

#include "_prog.h"
