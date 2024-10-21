#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# ffb.S
#-----------------------------------------------------------------------------

#include "riscv_test.h"
#include "test_macros.h"
#include "andestar_test.h"
#include "andestar_test_macros.h"

RVTEST_RV64U
ASTAR_CODE_BEGIN

  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  # Test all mismatch
  ASTAR_TEST_RR_OP( 2,  ffb,  0, 0x8877665544332211, 0x00 );
  ASTAR_TEST_RR_OP( 3,  ffb,  0, 0x8877665544332211, 0xaa );

  # Test one match
  ASTAR_TEST_RR_OP( 4,  ffb, -8, 0x8877665544332211, 0x11 );
  ASTAR_TEST_RR_OP( 5,  ffb, -7, 0x8877665544332211, 0x22 );
  ASTAR_TEST_RR_OP( 6,  ffb, -6, 0x8877665544332211, 0x33 );
  ASTAR_TEST_RR_OP( 7,  ffb, -5, 0x8877665544332211, 0x44 );
  ASTAR_TEST_RR_OP( 8,  ffb, -4, 0x8877665544332211, 0x55 );
  ASTAR_TEST_RR_OP( 9,  ffb, -3, 0x8877665544332211, 0x66 );
  ASTAR_TEST_RR_OP(10,  ffb, -2, 0x8877665544332211, 0x77 );
  ASTAR_TEST_RR_OP(11,  ffb, -1, 0x8877665544332211, 0x88 );

    # Test two match
  ASTAR_TEST_RR_OP(12,  ffb, -8, 0x8877665544331111, 0x11 );
  ASTAR_TEST_RR_OP(13,  ffb, -7, 0x8877665544222211, 0x22 );
  ASTAR_TEST_RR_OP(14,  ffb, -6, 0x8877665533332211, 0x33 );
  ASTAR_TEST_RR_OP(15,  ffb, -5, 0x8877664444332211, 0x44 );
  ASTAR_TEST_RR_OP(16,  ffb, -4, 0x8877555544332211, 0x55 );
  ASTAR_TEST_RR_OP(17,  ffb, -3, 0x8866665544332211, 0x66 );
  ASTAR_TEST_RR_OP(18,  ffb, -2, 0x7777665544332211, 0x77 );

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END
