#*****************************************************************************
# fminm_s.S
#-----------------------------------------------------------------------------
#
# Test f{min|max}m.s instructinos.
#

#include "riscv_test.h"
#include "test_macros.h"

RVTEST_RV64UF
RVTEST_CODE_BEGIN

  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  TEST_FP_OP2_S( 2,  fminm.s, 0,        1.0,        2.5,        1.0 );
  TEST_FP_OP2_S( 3,  fminm.s, 0,    -1235.1,    -1235.1,        1.1 );
  TEST_FP_OP2_S( 4,  fminm.s, 0,    -1235.1,        1.1,    -1235.1 );
  TEST_FP_OP2_S( 5,  fminm.s, 0,    -1235.1,        NaN,    -1235.1 );
  TEST_FP_OP2_S( 6,  fminm.s, 0, 0.00000001, 3.14159265, 0.00000001 );
  TEST_FP_OP2_S( 7,  fminm.s, 0,       -2.0,       -1.0,       -2.0 );

  TEST_FP_OP2_S(12,  fmaxm.s, 0,        2.5,        2.5,        1.0 );
  TEST_FP_OP2_S(13,  fmaxm.s, 0,        1.1,    -1235.1,        1.1 );
  TEST_FP_OP2_S(14,  fmaxm.s, 0,        1.1,        1.1,    -1235.1 );
  TEST_FP_OP2_S(15,  fmaxm.s, 0,    -1235.1,        NaN,    -1235.1 );
  TEST_FP_OP2_S(16,  fmaxm.s, 0, 3.14159265, 3.14159265, 0.00000001 );
  TEST_FP_OP2_S(17,  fmaxm.s, 0,       -1.0,       -1.0,       -2.0 );

  # FMAX(sNaN, x) = x
  TEST_FP_OP2_S(20,  fmaxm.s, 0x10, 1.0, sNaNf, 1.0);
  # FMAX(qNaN, qNaN) = canonical NaN
  TEST_FP_OP2_S(21,  fmaxm.s, 0x00, qNaNf, NaN, NaN);

  # -0.0 < +0.0
  TEST_FP_OP2_S(30,  fminm.s, 0,       -0.0,       -0.0,        0.0 );
  TEST_FP_OP2_S(31,  fminm.s, 0,       -0.0,        0.0,       -0.0 );
  TEST_FP_OP2_S(32,  fmaxm.s, 0,        0.0,       -0.0,        0.0 );
  TEST_FP_OP2_S(33,  fmaxm.s, 0,        0.0,        0.0,       -0.0 );

  TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END

