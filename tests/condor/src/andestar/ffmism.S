#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# ffmism.S
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
  ASTAR_TEST_RR_OP( 1,  ffmism,  0, 0x8877665544332211, 0x8877665544332211 );

  # Test all mismatch
  ASTAR_TEST_RR_OP( 2,  ffmism, -8, 0x8877665544332211, 0x1122334455667788 );

  # Test all zeros
  ASTAR_TEST_RR_OP( 3,  ffmism,  0, 0x0000000000000000, 0x0000000000000000 );  

  # Test one mismatch
  ASTAR_TEST_RR_OP(18,  ffmism, -8, 0x8877665544332211, 0x88776655443322aa );
  ASTAR_TEST_RR_OP(19,  ffmism, -7, 0x8877665544332211, 0x887766554433aa11 );
  ASTAR_TEST_RR_OP(20,  ffmism, -6, 0x8877665544332211, 0x8877665544aa2211 );
  ASTAR_TEST_RR_OP(21,  ffmism, -5, 0x8877665544332211, 0x88776655aa332211 );
  ASTAR_TEST_RR_OP(22,  ffmism, -4, 0x8877665544332211, 0x887766aa44332211 );
  ASTAR_TEST_RR_OP(23,  ffmism, -3, 0x8877665544332211, 0x8877aa5544332211 );
  ASTAR_TEST_RR_OP(24,  ffmism, -2, 0x8877665544332211, 0x88aa665544332211 );
  ASTAR_TEST_RR_OP(25,  ffmism, -1, 0x8877665544332211, 0xaa77665544332211 );

  # Test two mismatch
  ASTAR_TEST_RR_OP(26,  ffmism, -8, 0xaa77665544332211, 0x88776655443322aa );
  ASTAR_TEST_RR_OP(27,  ffmism, -7, 0x88aa665544332211, 0x887766554433aa11 );
  ASTAR_TEST_RR_OP(28,  ffmism, -6, 0x8877aa5544332211, 0x8877665544aa2211 );
  ASTAR_TEST_RR_OP(29,  ffmism, -5, 0x887766aa44332211, 0x88776655aa332211 );
  ASTAR_TEST_RR_OP(30,  ffmism, -4, 0x8877aa5544332211, 0x887766aa44332211 );
  ASTAR_TEST_RR_OP(31,  ffmism, -3, 0x88aa665544332211, 0x8877aa5544332211 );
  ASTAR_TEST_RR_OP(32,  ffmism, -2, 0xaa77665544332211, 0x88aa665544332211 );

  # Test one zero match and one mismatch
  ASTAR_TEST_RR_OP(33,  ffmism, -1, 0x8877665544332200, 0xaa77665544332200 );
  ASTAR_TEST_RR_OP(34,  ffmism, -2, 0x8877665544330011, 0x88aa665544330011 );
  ASTAR_TEST_RR_OP(35,  ffmism, -3, 0x8877665544002211, 0x8877aa5544002211 );
  ASTAR_TEST_RR_OP(36,  ffmism, -4, 0x8877665500332211, 0x887766aa00332211 );
  ASTAR_TEST_RR_OP(37,  ffmism, -3, 0x8877660044332211, 0x8877aa0044332211 );
  ASTAR_TEST_RR_OP(38,  ffmism, -2, 0x8877005544332211, 0x88aa005544332211 );
  ASTAR_TEST_RR_OP(39,  ffmism, -1, 0x8800665544332211, 0xaa00665544332211 );

  # Test one mismatch and one zero match
  ASTAR_TEST_RR_OP(40,  ffmism, -8, 0x0077665544332211, 0x00776655443322aa );
  ASTAR_TEST_RR_OP(41,  ffmism, -7, 0x8800665544332211, 0x880066554433aa11 );
  ASTAR_TEST_RR_OP(42,  ffmism, -6, 0x8877005544332211, 0x8877005544aa2211 );
  ASTAR_TEST_RR_OP(43,  ffmism, -5, 0x8877660044332211, 0x88776600aa332211 );
  ASTAR_TEST_RR_OP(44,  ffmism, -4, 0x8877005544332211, 0x887700aa44332211 );
  ASTAR_TEST_RR_OP(45,  ffmism, -3, 0x8800665544332211, 0x8800aa5544332211 );
  ASTAR_TEST_RR_OP(46,  ffmism, -2, 0x0077665544332211, 0x00aa665544332211 );

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END
