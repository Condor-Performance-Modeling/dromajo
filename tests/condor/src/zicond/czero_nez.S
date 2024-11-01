#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# czero_nez.S
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

  ASTAR_TEST_RR_OP( 1,  czero.nez,  0x8877665544332211, 0x8877665544332211, 0x0000000000000000 );

  ASTAR_TEST_RR_OP( 2,  czero.nez,  0x0000000000000000, 0x8877665544332211, 0x0000000000000001 );
  ASTAR_TEST_RR_OP( 3,  czero.nez,  0x0000000000000000, 0x8877665544332211, 0x0000000000000100 );
  ASTAR_TEST_RR_OP( 4,  czero.nez,  0x0000000000000000, 0x8877665544332211, 0x0000000000010000 );
  ASTAR_TEST_RR_OP( 5,  czero.nez,  0x0000000000000000, 0x8877665544332211, 0x0000000001000000 );
  ASTAR_TEST_RR_OP( 6,  czero.nez,  0x0000000000000000, 0x8877665544332211, 0x0000000100000000 );
  ASTAR_TEST_RR_OP( 7,  czero.nez,  0x0000000000000000, 0x8877665544332211, 0x0000010000000000 );
  ASTAR_TEST_RR_OP( 8,  czero.nez,  0x0000000000000000, 0x8877665544332211, 0x0001000000000000 );
  ASTAR_TEST_RR_OP( 9,  czero.nez,  0x0000000000000000, 0x8877665544332211, 0x0100000000000000 );

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END
