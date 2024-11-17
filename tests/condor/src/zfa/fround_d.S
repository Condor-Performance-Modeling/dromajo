#*****************************************************************************
# fround_d.S
#-----------------------------------------------------------------------------

#include "riscv_test.h"
#include "test_macros.h"

RVTEST_RV64UF
RVTEST_CODE_BEGIN

#  nop
#  TEST_PASSFAIL

  RVTEST_FAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END

