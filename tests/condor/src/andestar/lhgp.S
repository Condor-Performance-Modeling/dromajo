#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# lhgp.S
#
# FIXME: this is copied from lh.S, needs to be converted to lhgp.S
#-----------------------------------------------------------------------------

#include "riscv_test.h"
#include "test_macros.h"
#include "andestar_test.h"
#include "andestar_test_macros.h"

RVTEST_RV64U
ASTAR_CODE_BEGIN

  #-------------------------------------------------------------
  # Basic tests
  #-------------------------------------------------------------

  ASTAR_TEST_LD_OP( 2, lhgp, 0x00000000000000ff, 0,  tdat );
  ASTAR_TEST_LD_OP( 3, lhgp, 0xffffffffffffff00, 2,  tdat );
  ASTAR_TEST_LD_OP( 4, lhgp, 0x0000000000000ff0, 4,  tdat );
  ASTAR_TEST_LD_OP( 5, lhgp, 0xfffffffffffff00f, 6,  tdat );

  # Test with negative offset

  ASTAR_TEST_LD_OP( 6, lhgp, 0x00000000000000ff, -6,  tdat4 );
  ASTAR_TEST_LD_OP( 7, lhgp, 0xffffffffffffff00, -4,  tdat4 );
  ASTAR_TEST_LD_OP( 8, lhgp, 0x0000000000000ff0, -2,  tdat4 );
  ASTAR_TEST_LD_OP( 9, lhgp, 0xfffffffffffff00f,  0,  tdat4 );

  # Test with a negative base

  ASTAR_TEST_CASE( 10, x5, 0x00000000000000ff, \
    la   x3, tdat; \
    addi x3, x3, -32; \
    lhgp x5, 32; \
  )

#  # Test with unaligned base
#
#  TEST_CASE( 11, x5, 0xffffffffffffff00, \
#    la  x1, tdat; \
#    addi x1, x1, -5; \
#    lh x5, 7(x1); \
#  )
#
#  #-------------------------------------------------------------
#  # Bypassing tests
#  #-------------------------------------------------------------
#
#  TEST_LD_DEST_BYPASS( 12, 0, lh, 0x0000000000000ff0, 2, tdat2 );
#  TEST_LD_DEST_BYPASS( 13, 1, lh, 0xfffffffffffff00f, 2, tdat3 );
#  TEST_LD_DEST_BYPASS( 14, 2, lh, 0xffffffffffffff00, 2, tdat1 );
#
#  TEST_LD_SRC1_BYPASS( 15, 0, lh, 0x0000000000000ff0, 2, tdat2 );
#  TEST_LD_SRC1_BYPASS( 16, 1, lh, 0xfffffffffffff00f, 2, tdat3 );
#  TEST_LD_SRC1_BYPASS( 17, 2, lh, 0xffffffffffffff00, 2, tdat1 );
#
#  #-------------------------------------------------------------
#  # Test write-after-write hazard
#  #-------------------------------------------------------------
#
#  TEST_CASE( 18, x2, 2, \
#    la  x5, tdat; \
#    lh  x2, 0(x5); \
#    li  x2, 2; \
#  )
#
#  TEST_CASE( 19, x2, 2, \
#    la  x5, tdat; \
#    lh  x2, 0(x5); \
#    nop; \
#    li  x2, 2; \
#  )

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

tdat:
tdat1:  .half 0x00ff
tdat2:  .half 0xff00
tdat3:  .half 0x0ff0
tdat4:  .half 0xf00f

RVTEST_DATA_END
