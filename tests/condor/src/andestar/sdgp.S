#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# sdgp.S
#
# FIXME: this is copied from sd.S, needs to be converted to sdgp.S
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

  ASTAR_TEST_ST_OP( 2, ldgp, sdgp, 0x00aa00aa00aa00aa,   0, tdat );
  ASTAR_TEST_ST_OP( 3, ldgp, sdgp, 0xaa00aa00aa00aa00,   8, tdat );
  ASTAR_TEST_ST_OP( 4, ldgp, sdgp, 0x0aa00aa00aa00aa0,  16, tdat );
  ASTAR_TEST_ST_OP( 5, ldgp, sdgp, 0xa00aa00aa00aa00a,  24, tdat );

  # Test with negative offset

  ASTAR_TEST_ST_OP( 6, ldgp, sdgp, 0x00aa00aa00aa00aa, -24, tdat8 );
  ASTAR_TEST_ST_OP( 7, ldgp, sdgp, 0xaa00aa00aa00aa00, -16, tdat8 );
  ASTAR_TEST_ST_OP( 8, ldgp, sdgp, 0x0aa00aa00aa00aa0,  -8, tdat8 );
  ASTAR_TEST_ST_OP( 9, ldgp, sdgp, 0xa00aa00aa00aa00a,   0, tdat8 );

#  # Test with a negative base

  ASTAR_TEST_CASE( 10, x5, 0x1234567812345678, \
    la   x3, tdat9; \
    li   x2, 0x1234567812345678; \
    addi x3, x3, -32; \
    sdgp x2, 32; \
    ldgp x5, 32; \
  )

#  # Test with unaligned base
#
#  TEST_CASE( 11, x5, 0x5821309858213098, \
#    la  x1, tdat9; \
#    li  x2, 0x5821309858213098; \
#    addi x1, x1, -3; \
#    sd x2, 11(x1); \
#    la  x4, tdat10; \
#    ld x5, 0(x4); \
#  )
#
#  #-------------------------------------------------------------
#  # Bypassing tests
#  #-------------------------------------------------------------
#
#  TEST_ST_SRC12_BYPASS( 12, 0, 0, ld, sd, 0xabbccdd, 0,  tdat );
#  TEST_ST_SRC12_BYPASS( 13, 0, 1, ld, sd, 0xaabbccd, 8,  tdat );
#  TEST_ST_SRC12_BYPASS( 14, 0, 2, ld, sd, 0xdaabbcc, 16, tdat );
#  TEST_ST_SRC12_BYPASS( 15, 1, 0, ld, sd, 0xddaabbc, 24, tdat );
#  TEST_ST_SRC12_BYPASS( 16, 1, 1, ld, sd, 0xcddaabb, 32, tdat );
#  TEST_ST_SRC12_BYPASS( 17, 2, 0, ld, sd, 0xccddaab, 40, tdat );
#
#  TEST_ST_SRC21_BYPASS( 18, 0, 0, ld, sd, 0x00112233, 0,  tdat );
#  TEST_ST_SRC21_BYPASS( 19, 0, 1, ld, sd, 0x30011223, 8,  tdat );
#  TEST_ST_SRC21_BYPASS( 20, 0, 2, ld, sd, 0x33001122, 16, tdat );
#  TEST_ST_SRC21_BYPASS( 21, 1, 0, ld, sd, 0x23300112, 24, tdat );
#  TEST_ST_SRC21_BYPASS( 22, 1, 1, ld, sd, 0x22330011, 32, tdat );
#  TEST_ST_SRC21_BYPASS( 23, 2, 0, ld, sd, 0x12233001, 40, tdat );

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

tdat:
tdat1:  .dword 0xdeadbeefdeadbeef
tdat2:  .dword 0xdeadbeefdeadbeef
tdat3:  .dword 0xdeadbeefdeadbeef
tdat4:  .dword 0xdeadbeefdeadbeef
tdat5:  .dword 0xdeadbeefdeadbeef
tdat6:  .dword 0xdeadbeefdeadbeef
tdat7:  .dword 0xdeadbeefdeadbeef
tdat8:  .dword 0xdeadbeefdeadbeef
tdat9:  .dword 0xdeadbeefdeadbeef
tdat10: .dword 0xdeadbeefdeadbeef

RVTEST_DATA_END
