#*****************************************************************************
# fminm_h.S
#-----------------------------------------------------------------------------
#
# Test f{min|max}m.h instructinos.
#

#include "riscv_test.h"
#include "test_macros.h"

RVTEST_RV64UF
RVTEST_CODE_BEGIN

  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  TEST_FP_OP2_H( 2,  fminm.h, 0,        1.0,        2.5,        1.0 );
  TEST_FP_OP2_H( 3,  fminm.h, 0,    -1235.1,    -1235.1,        1.1 );
  TEST_FP_OP2_H( 4,  fminm.h, 0,    -1235.1,        1.1,    -1235.1 );
  TEST_FP_OP2_H( 5,  fminm.h, 0,    -1235.1,        NaN,    -1235.1 );
  TEST_FP_OP2_H( 6,  fminm.h, 0, 0.00000001, 3.14159265, 0.00000001 );
  TEST_FP_OP2_H( 7,  fminm.h, 0,       -2.0,       -1.0,       -2.0 );

  TEST_FP_OP2_H(12,  fmaxm.h, 0,        2.5,        2.5,        1.0 );
  TEST_FP_OP2_H(13,  fmaxm.h, 0,        1.1,    -1235.1,        1.1 );
  TEST_FP_OP2_H(14,  fmaxm.h, 0,        1.1,        1.1,    -1235.1 );
  TEST_FP_OP2_H(15,  fmaxm.h, 0,    -1235.1,        NaN,    -1235.1 );
  TEST_FP_OP2_H(16,  fmaxm.h, 0, 3.14159265, 3.14159265, 0.00000001 );
  TEST_FP_OP2_H(17,  fmaxm.h, 0,       -1.0,       -1.0,       -2.0 );

  # FMIN(hNaN, x) = x
  TEST_FP_OP2_H(20,  fmaxm.h, 0x10, 1.0, sNaNh, 1.0);
  # FMIN(hNaN, hNaN) = canonical NaN
  TEST_FP_OP2_H(21,  fmaxm.h, 0x00, qNaNh, NaN, NaN);

  # -0.0 < +0.0
  TEST_FP_OP2_H(30,  fminm.h, 0,       -0.0,       -0.0,        0.0 );
  TEST_FP_OP2_H(31,  fminm.h, 0,       -0.0,        0.0,       -0.0 );
  TEST_FP_OP2_H(32,  fmaxm.h, 0,        0.0,       -0.0,        0.0 );
  TEST_FP_OP2_H(33,  fmaxm.h, 0,        0.0,        0.0,       -0.0 );

  TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END

