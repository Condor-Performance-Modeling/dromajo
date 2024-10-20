#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# lwgp.S
#
# FIXME: this is copied from lw.S, needs to be converted to lwgp.S
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
                    #  instr expect              offset  base symbol
  ASTAR_TEST_LD_OP( 2, lwgp, 0x0000000000ff00ff,  0,      tdat );
  ASTAR_TEST_LD_OP( 3, lwgp, 0xffffffffff00ff00,  4,      tdat );
  ASTAR_TEST_LD_OP( 4, lwgp, 0x000000000ff00ff0,  8,      tdat );
  ASTAR_TEST_LD_OP( 5, lwgp, 0xfffffffff00ff00f, 12,      tdat );

  # Test with negative offset

  ASTAR_TEST_LD_OP( 6, lwgp, 0x0000000000ff00ff, -12,  tdat4 );
  ASTAR_TEST_LD_OP( 7, lwgp, 0xffffffffff00ff00,  -8,  tdat4 );
  ASTAR_TEST_LD_OP( 8, lwgp, 0x000000000ff00ff0,  -4,  tdat4 );
  ASTAR_TEST_LD_OP( 9, lwgp, 0xfffffffff00ff00f,   0,  tdat4 );

  # Test with a negative base

  ASTAR_TEST_CASE( 10, x5, 0x0000000000ff00ff, \
    la  x3, tdat; \
    addi x3, x3, -32; \
    lwgp x5, 32; \
  )

  # Test with unaligned base

#  ASTAR_TEST_CASE( 11, x5, 0xffffffffff00ff00, \
#    la   x3, tdat; \
#    addi x3, x3, -3; \
#    lwgp x5, 7; \
#  )
#
#  #-------------------------------------------------------------
#  # Bypassing tests
#  #-------------------------------------------------------------
#
#  TEST_LD_DEST_BYPASS( 12, 0, lw, 0x000000000ff00ff0, 4, tdat2 );
#  TEST_LD_DEST_BYPASS( 13, 1, lw, 0xfffffffff00ff00f, 4, tdat3 );
#  TEST_LD_DEST_BYPASS( 14, 2, lw, 0xffffffffff00ff00, 4, tdat1 );
#
#  TEST_LD_SRC1_BYPASS( 15, 0, lw, 0x000000000ff00ff0, 4, tdat2 );
#  TEST_LD_SRC1_BYPASS( 16, 1, lw, 0xfffffffff00ff00f, 4, tdat3 );
#  TEST_LD_SRC1_BYPASS( 17, 2, lw, 0xffffffffff00ff00, 4, tdat1 );
#
#  #-------------------------------------------------------------
#  # Test write-after-write hazard
#  #-------------------------------------------------------------
#
#  TEST_CASE( 18, x2, 2, \
#    la  x5, tdat; \
#    lw  x2, 0(x5); \
#    li  x2, 2; \
#  )
#
#  TEST_CASE( 19, x2, 2, \
#    la  x5, tdat; \
#    lw  x2, 0(x5); \
#    nop; \
#    li  x2, 2; \
#  )

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

tdat:
tdat1:  .word 0x00ff00ff
tdat2:  .word 0xff00ff00
tdat3:  .word 0x0ff00ff0
tdat4:  .word 0xf00ff00f

RVTEST_DATA_END
