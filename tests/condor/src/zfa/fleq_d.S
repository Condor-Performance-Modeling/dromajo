# See LICENSE for license details.

#*****************************************************************************
# fleq_d.S
#-----------------------------------------------------------------------------
#
# Test f{min|max}.h instructinos.
#

#include "riscv_test.h"
#include "test_macros.h"

RVTEST_RV64UF
RVTEST_CODE_BEGIN

  #-------------------------------------------------------------
  # This is a template - is not called
  #-------------------------------------------------------------

#  TEST_FP_OP2_D( 2,  fleq.d, 0,        1.0,        2.5,        1.0 );

  TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END
