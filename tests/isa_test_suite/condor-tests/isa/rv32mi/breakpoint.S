# See LICENSE for license details.

#include "test_macros.h"
#undef RVTEST_RV64M
#define RVTEST_RV64M RVTEST_RV32M
#define __MACHINE_MODE

#include "../rv64mi/breakpoint.S"
