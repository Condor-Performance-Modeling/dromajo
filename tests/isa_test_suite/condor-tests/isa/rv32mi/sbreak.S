# See LICENSE for license details.

#include "test_macros.h"
#undef RVTEST_RV64S
#define RVTEST_RV64S RVTEST_RV32M
#define __MACHINE_MODE

#include "../rv64si/sbreak.S"
