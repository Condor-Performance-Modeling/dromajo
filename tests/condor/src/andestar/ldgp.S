#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# ldgp.S
#
# FIXME: this is copied from ld.S, needs to be converted to ldgp.S
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

  ASTAR_TEST_LD_OP( 2, ldgp, 0x00ff00ff00ff00ff, 0,  tdat );
  ASTAR_TEST_LD_OP( 3, ldgp, 0xff00ff00ff00ff00, 8,  tdat );
  ASTAR_TEST_LD_OP( 4, ldgp, 0x0ff00ff00ff00ff0, 16, tdat );
  ASTAR_TEST_LD_OP( 5, ldgp, 0xf00ff00ff00ff00f, 24, tdat );

  # Test with negative offset

  ASTAR_TEST_LD_OP( 6, ldgp, 0x00ff00ff00ff00ff, -24, tdat4 );
  ASTAR_TEST_LD_OP( 7, ldgp, 0xff00ff00ff00ff00, -16, tdat4 );
  ASTAR_TEST_LD_OP( 8, ldgp, 0x0ff00ff00ff00ff0, -8,  tdat4 );
  ASTAR_TEST_LD_OP( 9, ldgp, 0xf00ff00ff00ff00f, 0,   tdat4 );

  # Test with a negative base

  ASTAR_TEST_CASE( 10, x5, 0x00ff00ff00ff00ff, \
    la   x3, tdat; \
    addi x3, x3, -32; \
    ldgp x5, 32; \
  )

  # Test with unaligned base

#  TEST_CASE( 11, x5, 0xff00ff00ff00ff00, \
#    la  x1, tdat; \
#    addi x1, x1, -3; \
#    ld x5, 11(x1); \
#  )
#
#  #-------------------------------------------------------------
#  # Bypassing tests
#  #-------------------------------------------------------------
#
#  TEST_LD_DEST_BYPASS( 12, 0, ld, 0x0ff00ff00ff00ff0, 8, tdat2 );
#  TEST_LD_DEST_BYPASS( 13, 1, ld, 0xf00ff00ff00ff00f, 8, tdat3 );
#  TEST_LD_DEST_BYPASS( 14, 2, ld, 0xff00ff00ff00ff00, 8, tdat1 );
#
#  TEST_LD_SRC1_BYPASS( 15, 0, ld, 0x0ff00ff00ff00ff0, 8, tdat2 );
#  TEST_LD_SRC1_BYPASS( 16, 1, ld, 0xf00ff00ff00ff00f, 8, tdat3 );
#  TEST_LD_SRC1_BYPASS( 17, 2, ld, 0xff00ff00ff00ff00, 8, tdat1 );
#
#  #-------------------------------------------------------------
#  # Test write-after-write hazard
#  #-------------------------------------------------------------
#
#  TEST_CASE( 18, x2, 2, \
#    la  x5, tdat; \
#    ld  x2, 0(x5); \
#    li  x2, 2; \
#  )
#
#  TEST_CASE( 19, x2, 2, \
#    la  x5, tdat; \
#    ld  x2, 0(x5); \
#    nop; \
#    li  x2, 2; \
#  )

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

tdat:
tdat1:  .dword 0x00ff00ff00ff00ff
tdat2:  .dword 0xff00ff00ff00ff00
tdat3:  .dword 0x0ff00ff00ff00ff0
tdat4:  .dword 0xf00ff00ff00ff00f

RVTEST_DATA_END
