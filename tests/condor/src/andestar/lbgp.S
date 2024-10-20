#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# lbgp.S
#
# FIXME: this is copied from lb.S, needs to be converted to lbgp.S
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

  ASTAR_TEST_LD_OP(  2, lbgp, 0xffffffffffffffff, 0,  tdat );
  ASTAR_TEST_LD_OP(  3, lbgp, 0x0000000000000000, 1,  tdat );
  ASTAR_TEST_LD_OP(  4, lbgp, 0xfffffffffffffff0, 2,  tdat );
  ASTAR_TEST_LD_OP(  5, lbgp, 0x000000000000000f, 3,  tdat );

  # Test with negative offset

  ASTAR_TEST_LD_OP(  6, lbgp, 0xffffffffffffffff, -3,  tdat4 );
  ASTAR_TEST_LD_OP(  7, lbgp, 0x0000000000000000, -2,  tdat4 );
  ASTAR_TEST_LD_OP(  8, lbgp, 0xfffffffffffffff0, -1,  tdat4 );
  ASTAR_TEST_LD_OP(  9, lbgp, 0x000000000000000f,  0,  tdat4 );

  # Test with a negative base

  ASTAR_TEST_CASE( 10, x5, 0xffffffffffffffff, \
    la   x3, tdat; \
    addi x3, x3, -32; \
    lbgp x5, 32; \
  )

  # Test with unaligned base

#  ASTAR_TEST_CASE( 11, x5, 0x0000000000000000, \
#    la   x3, tdat; \
#    addi x3, x3, -6; \
#    lbgp x5, 7; \
#  )
#
#  #-------------------------------------------------------------
#  # Bypassing tests
#  #-------------------------------------------------------------
#
#  TEST_LD_DEST_BYPASS( 12, 0, lb, 0xfffffffffffffff0, 1, tdat2 );
#  TEST_LD_DEST_BYPASS( 13, 1, lb, 0x000000000000000f, 1, tdat3 );
#  TEST_LD_DEST_BYPASS( 14, 2, lb, 0x0000000000000000, 1, tdat1 );
#
#  TEST_LD_SRC1_BYPASS( 15, 0, lb, 0xfffffffffffffff0, 1, tdat2 );
#  TEST_LD_SRC1_BYPASS( 16, 1, lb, 0x000000000000000f, 1, tdat3 );
#  TEST_LD_SRC1_BYPASS( 17, 2, lb, 0x0000000000000000, 1, tdat1 );
#
#  #-------------------------------------------------------------
#  # Test write-after-write hazard
#  #-------------------------------------------------------------
#
#  TEST_CASE( 18, x2, 2, \
#    la  x5, tdat; \
#    lb  x2, 0(x5); \
#    li  x2, 2; \
#  )
#
#  TEST_CASE( 19, x2, 2, \
#    la  x5, tdat; \
#    lb  x2, 0(x5); \
#    nop; \
#    li  x2, 2; \
#  )

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

tdat:
tdat1:  .byte 0xff
tdat2:  .byte 0x00
tdat3:  .byte 0xf0
tdat4:  .byte 0x0f

RVTEST_DATA_END
