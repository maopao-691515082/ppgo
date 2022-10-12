#pragma once

#include "_conf.h"

#include "_internal.h"
#include "_mem.h"
#include "_base_type.h"
#include "_exc.h"
#include "_with.h"
#include "_util.h"

namespace ppgo
{

void SetArgs(int argc, char *argv[]);
Exc::Ptr main();

}

#include "_prog.h"
