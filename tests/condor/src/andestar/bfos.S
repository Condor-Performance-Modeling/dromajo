#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# bfos.S
#-----------------------------------------------------------------------------

#include "riscv_test.h"
#include "test_macros.h"
#include "andestar_test.h"
#include "andestar_test_macros.h"

RVTEST_RV64U
ASTAR_CODE_BEGIN

# msb == 0
#                       inst,             expect,                val, msb, lsb
  ASTAR_TEST_BFO_OP( 0, bfos, 0xffffffffffffffff, 0xffffffffffffffff,   0,   0)
  ASTAR_TEST_BFO_OP( 1, bfos, 0xffffffffffff8000, 0xffffffffffffffff,   0,  15)
  ASTAR_TEST_BFO_OP( 2, bfos, 0xffffffffffff0000, 0xffffffffffffffff,   0,  16)
  ASTAR_TEST_BFO_OP( 3, bfos, 0xffffffff80000000, 0xffffffffffffffff,   0,  31)
  ASTAR_TEST_BFO_OP( 4, bfos, 0xffffffff00000000, 0xffffffffffffffff,   0,  32)
  ASTAR_TEST_BFO_OP( 5, bfos, 0xffff800000000000, 0xffffffffffffffff,   0,  47)
  ASTAR_TEST_BFO_OP( 6, bfos, 0xffff000000000000, 0xffffffffffffffff,   0,  48)
  ASTAR_TEST_BFO_OP( 7, bfos, 0x8000000000000000, 0xffffffffffffffff,   0,  63)

  ASTAR_TEST_BFO_OP(10, bfos, 0x0000000000000000, 0xfffffffffffffffe,   0,   0)
  ASTAR_TEST_BFO_OP(11, bfos, 0x0000000000000000, 0xfffffffffffffffe,   0,  15)
  ASTAR_TEST_BFO_OP(12, bfos, 0x0000000000000000, 0xfffffffffffffffe,   0,  16)
  ASTAR_TEST_BFO_OP(13, bfos, 0x0000000000000000, 0xfffffffffffffffe,   0,  31)
  ASTAR_TEST_BFO_OP(14, bfos, 0x0000000000000000, 0xfffffffffffffffe,   0,  32)
  ASTAR_TEST_BFO_OP(15, bfos, 0x0000000000000000, 0xfffffffffffffffe,   0,  47)
  ASTAR_TEST_BFO_OP(16, bfos, 0x0000000000000000, 0xfffffffffffffffe,   0,  48)
  ASTAR_TEST_BFO_OP(17, bfos, 0x0000000000000000, 0xfffffffffffffffe,   0,  63)

# msb < lsb
#                       inst,             expect,                val, msb, lsb
  ASTAR_TEST_BFO_OP(20, bfos, 0xbcdef01234567890, 0xabcdef0123456789,   4,  63)
  ASTAR_TEST_BFO_OP(21, bfos, 0xfdef012345678900, 0xabcdef0123456789,   8,  59)

# msb > lsb
#                       inst,             expect,                val, msb, lsb
  ASTAR_TEST_BFO_OP(30, bfos, 0xabcdef0123456789, 0xabcdef0123456789,  63,   0)
  ASTAR_TEST_BFO_OP(31, bfos, 0xffbcdef012345678, 0xabcdef0123456789,  59,   4)
  ASTAR_TEST_BFO_OP(32, bfos, 0x0000000000000000, 0xabcdef0123456789,   4,   4)
  ASTAR_TEST_BFO_OP(33, bfos, 0xffffffffffffffff, 0xabcdef0123456789,   8,   8)

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END
