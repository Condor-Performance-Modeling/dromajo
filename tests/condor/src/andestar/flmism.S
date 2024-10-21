#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# flmism.S
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

  # Test all match
  ASTAR_TEST_RR_OP( 1,  flmism,  0, 0x8877665544332211, 0x8877665544332211 );

  # Test all mismatch
  ASTAR_TEST_RR_OP( 2,  flmism, -1, 0x8877665544332211, 0x1122334455667788 );

  # Test all zeros
  ASTAR_TEST_RR_OP( 3,  flmism,  0, 0x0000000000000000, 0x0000000000000000 );  

  # Test one mismatch
  ASTAR_TEST_RR_OP(18,  flmism, -8, 0x8877665544332211, 0x88776655443322aa );
  ASTAR_TEST_RR_OP(19,  flmism, -7, 0x8877665544332211, 0x887766554433aa11 );
  ASTAR_TEST_RR_OP(20,  flmism, -6, 0x8877665544332211, 0x8877665544aa2211 );
  ASTAR_TEST_RR_OP(21,  flmism, -5, 0x8877665544332211, 0x88776655aa332211 );
  ASTAR_TEST_RR_OP(22,  flmism, -4, 0x8877665544332211, 0x887766aa44332211 );
  ASTAR_TEST_RR_OP(23,  flmism, -3, 0x8877665544332211, 0x8877aa5544332211 );
  ASTAR_TEST_RR_OP(24,  flmism, -2, 0x8877665544332211, 0x88aa665544332211 );
  ASTAR_TEST_RR_OP(25,  flmism, -1, 0x8877665544332211, 0xaa77665544332211 );

  # Test two mismatch
  ASTAR_TEST_RR_OP(26,  flmism, -1, 0xaa77665544332211, 0x88776655443322aa );
  ASTAR_TEST_RR_OP(27,  flmism, -2, 0x88aa665544332211, 0x887766554433aa11 );
  ASTAR_TEST_RR_OP(28,  flmism, -3, 0x8877aa5544332211, 0x8877665544aa2211 );
  ASTAR_TEST_RR_OP(29,  flmism, -4, 0x887766aa44332211, 0x88776655aa332211 );
  ASTAR_TEST_RR_OP(30,  flmism, -3, 0x8877aa5544332211, 0x887766aa44332211 );
  ASTAR_TEST_RR_OP(31,  flmism, -2, 0x88aa665544332211, 0x8877aa5544332211 );
  ASTAR_TEST_RR_OP(32,  flmism, -1, 0xaa77665544332211, 0x88aa665544332211 );

  # Test one zero match and one mismatch
  ASTAR_TEST_RR_OP(33,  flmism, -1, 0x8877665544332200, 0xaa77665544332200 );
  ASTAR_TEST_RR_OP(34,  flmism, -2, 0x8877665544330011, 0x88aa665544330011 );
  ASTAR_TEST_RR_OP(35,  flmism, -3, 0x8877665544002211, 0x8877aa5544002211 );
  ASTAR_TEST_RR_OP(36,  flmism, -4, 0x8877665500332211, 0x887766aa00332211 );
  ASTAR_TEST_RR_OP(37,  flmism, -3, 0x8877660044332211, 0x8877aa0044332211 );
  ASTAR_TEST_RR_OP(38,  flmism, -2, 0x8877005544332211, 0x88aa005544332211 );
  ASTAR_TEST_RR_OP(39,  flmism, -1, 0x8800665544332211, 0xaa00665544332211 );

  # Test one mismatch and one zero match
  ASTAR_TEST_RR_OP(40,  flmism, -8, 0x0077665544332211, 0x00776655443322aa );
  ASTAR_TEST_RR_OP(41,  flmism, -7, 0x8800665544332211, 0x880066554433aa11 );
  ASTAR_TEST_RR_OP(42,  flmism, -6, 0x8877005544332211, 0x8877005544aa2211 );
  ASTAR_TEST_RR_OP(43,  flmism, -5, 0x8877660044332211, 0x88776600aa332211 );
  ASTAR_TEST_RR_OP(44,  flmism, -4, 0x8877005544332211, 0x887700aa44332211 );
  ASTAR_TEST_RR_OP(45,  flmism, -3, 0x8800665544332211, 0x8800aa5544332211 );
  ASTAR_TEST_RR_OP(46,  flmism, -2, 0x0077665544332211, 0x00aa665544332211 );

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END
