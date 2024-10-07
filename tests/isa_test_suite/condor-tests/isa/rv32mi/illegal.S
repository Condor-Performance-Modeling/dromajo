# See LICENSE for license details.

#include "test_macros.h"
#undef RVTEST_RV64M
#define RVTEST_RV64M RVTEST_RV32M

#include "../rv64mi/illegal.S"
