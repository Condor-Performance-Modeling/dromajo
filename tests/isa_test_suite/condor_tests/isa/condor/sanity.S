# See LICENSE for license details.

#*****************************************************************************
# sanity.S
#-----------------------------------------------------------------------------
#
# Basic sanity test to verify compilation and setup structure.
#

#include "riscv_test.h"
#include "test_macros.h"
RVTEST_RV64M
RVTEST_CODE_BEGIN

  # Simple no-operation code sequence to confirm the program structure.
  li t0, 0        # Load zero into t0 as a basic operation.
  li TESTNUM, 1   # Set TESTNUM to 1 to indicate test number 1.

  j pass          # Jump directly to pass.

  TEST_PASSFAIL   # Macro for handling pass/fail status.

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END