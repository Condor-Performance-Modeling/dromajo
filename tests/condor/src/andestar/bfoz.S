#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# bfoz.S
#-----------------------------------------------------------------------------

#include "riscv_test.h"
#include "test_macros.h"
#include "andestar_test.h"
#include "andestar_test_macros.h"

RVTEST_RV64U
ASTAR_CODE_BEGIN

# msb == 0
#                       inst,             expect,                val, msb, lsb
  ASTAR_TEST_BFO_OP( 0, bfoz, 0x0000000000000001, 0xffffffffffffffff,   0,   0)
  ASTAR_TEST_BFO_OP( 1, bfoz, 0x0000000000008000, 0xffffffffffffffff,   0,  15)
  ASTAR_TEST_BFO_OP( 2, bfoz, 0x0000000000010000, 0xffffffffffffffff,   0,  16)
  ASTAR_TEST_BFO_OP( 3, bfoz, 0x0000000080000000, 0xffffffffffffffff,   0,  31)
  ASTAR_TEST_BFO_OP( 4, bfoz, 0x0000000100000000, 0xffffffffffffffff,   0,  32)
  ASTAR_TEST_BFO_OP( 5, bfoz, 0x0000800000000000, 0xffffffffffffffff,   0,  47)
  ASTAR_TEST_BFO_OP( 6, bfoz, 0x0001000000000000, 0xffffffffffffffff,   0,  48)
  ASTAR_TEST_BFO_OP( 7, bfoz, 0x8000000000000000, 0xffffffffffffffff,   0,  63)

  ASTAR_TEST_BFO_OP(10, bfoz, 0x0000000000000000, 0xfffffffffffffffe,   0,   0)
  ASTAR_TEST_BFO_OP(11, bfoz, 0x0000000000000000, 0xfffffffffffffffe,   0,  15)
  ASTAR_TEST_BFO_OP(12, bfoz, 0x0000000000000000, 0xfffffffffffffffe,   0,  16)
  ASTAR_TEST_BFO_OP(13, bfoz, 0x0000000000000000, 0xfffffffffffffffe,   0,  31)
  ASTAR_TEST_BFO_OP(14, bfoz, 0x0000000000000000, 0xfffffffffffffffe,   0,  32)
  ASTAR_TEST_BFO_OP(15, bfoz, 0x0000000000000000, 0xfffffffffffffffe,   0,  47)
  ASTAR_TEST_BFO_OP(16, bfoz, 0x0000000000000000, 0xfffffffffffffffe,   0,  48)
  ASTAR_TEST_BFO_OP(17, bfoz, 0x0000000000000000, 0xfffffffffffffffe,   0,  63)

# msb < lsb
#                       inst,             expect,                val, msb, lsb
  ASTAR_TEST_BFO_OP(20, bfoz, 0xbcdef01234567890, 0xabcdef0123456789,   4,  63)
  ASTAR_TEST_BFO_OP(21, bfoz, 0x0def012345678900, 0xabcdef0123456789,   8,  59)

# msb >= lsb
#                       inst,             expect,                val, msb, lsb
  ASTAR_TEST_BFO_OP(30, bfoz, 0xabcdef0123456789, 0xabcdef0123456789,  63,   0)
  ASTAR_TEST_BFO_OP(31, bfoz, 0x00bcdef012345678, 0xabcdef0123456789,  59,   4)
  ASTAR_TEST_BFO_OP(32, bfoz, 0x0000000000000000, 0xabcdef0123456789,   4,   4)
  ASTAR_TEST_BFO_OP(33, bfoz, 0x0000000000000001, 0xabcdef0123456789,   8,   8)

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END
