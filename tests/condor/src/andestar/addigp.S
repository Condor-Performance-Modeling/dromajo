#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# addigp.S
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# | 31  | 30:21   | 20  | 19:17    | 16:15    | 14 | 13:12|11:7 |  6:0     |
# |-----|---------|-----|----------|----------|----|------|-----|----------|
# |imm17|imm[10:1]|imm11|imm[14:12]|imm[16:15]|imm0|ADDIGP| rd  | Custom-0 |
# |     |         |     |          |          |    |  01  |     | 0001011  |
#-----------------------------------------------------------------------------
# ASTAR_TEST_S18_OP(testnum, instruction, expect value, initial value, imm)
# In ASTAR_CODE_BEGIN TESTNUM is x8, normally it is x3
#
# x8  = test number
# x3  = is assigned val1
# x7  = expected value
# x14 = is RD of the op
# If x14 != x7 branch <fail>
#-----------------------------------------------------------------------------

#include "riscv_test.h"
#include "test_macros.h"
#include "andestar_test.h"
#include "andestar_test_macros.h"

RVTEST_RV64U
ASTAR_CODE_BEGIN

#                    #   instr   expect              x3                  imm
  ASTAR_TEST_S18_OP( 2,  addigp, 0x1000000000000000, 0x1000000000000000,  0x00000 );
  ASTAR_TEST_S18_OP( 3,  addigp, 0x1000000000000001, 0x1000000000000000,  0x00001 );
  ASTAR_TEST_S18_OP( 4,  addigp, 0x1000000000000002, 0x1000000000000000,  0x00002 );
  ASTAR_TEST_S18_OP( 5,  addigp, 0x1000000000000004, 0x1000000000000000,  0x00004 );
  ASTAR_TEST_S18_OP( 6,  addigp, 0x1000000000000008, 0x1000000000000000,  0x00008 );
  ASTAR_TEST_S18_OP( 7,  addigp, 0x1000000000000010, 0x1000000000000000,  0x00010 );
  ASTAR_TEST_S18_OP( 8,  addigp, 0x1000000000000020, 0x1000000000000000,  0x00020 );
  ASTAR_TEST_S18_OP( 9,  addigp, 0x1000000000000040, 0x1000000000000000,  0x00040 );
  ASTAR_TEST_S18_OP(10,  addigp, 0x1000000000000080, 0x1000000000000000,  0x00080 );
  ASTAR_TEST_S18_OP(11,  addigp, 0x1000000000000100, 0x1000000000000000,  0x00100 );
  ASTAR_TEST_S18_OP(12,  addigp, 0x1000000000000200, 0x1000000000000000,  0x00200 );
  ASTAR_TEST_S18_OP(13,  addigp, 0x1000000000000400, 0x1000000000000000,  0x00400 );
  ASTAR_TEST_S18_OP(14,  addigp, 0x1000000000000800, 0x1000000000000000,  0x00800 );
  ASTAR_TEST_S18_OP(15,  addigp, 0x1000000000001000, 0x1000000000000000,  0x01000 );
  ASTAR_TEST_S18_OP(16,  addigp, 0x1000000000002000, 0x1000000000000000,  0x02000 );
  ASTAR_TEST_S18_OP(17,  addigp, 0x1000000000004000, 0x1000000000000000,  0x04000 );
  ASTAR_TEST_S18_OP(18,  addigp, 0x1000000000008000, 0x1000000000000000,  0x08000 );
  ASTAR_TEST_S18_OP(19,  addigp, 0x1000000000010000, 0x1000000000000000,  0x10000 );

  ASTAR_TEST_S18_OP(20,  addigp, 0x1000000000000000, 0x1000000000000000,  0x00000 );
  ASTAR_TEST_S18_OP(21,  addigp, 0x0fffffffffffffff, 0x1000000000000000, -0x00001 );
  ASTAR_TEST_S18_OP(22,  addigp, 0x0ffffffffffffffe, 0x1000000000000000, -0x00002 );
  ASTAR_TEST_S18_OP(23,  addigp, 0x0ffffffffffffffc, 0x1000000000000000, -0x00004 );
  ASTAR_TEST_S18_OP(24,  addigp, 0x0ffffffffffffff8, 0x1000000000000000, -0x00008 );
  ASTAR_TEST_S18_OP(25,  addigp, 0x0ffffffffffffff0, 0x1000000000000000, -0x00010 );
  ASTAR_TEST_S18_OP(26,  addigp, 0x0fffffffffffffe0, 0x1000000000000000, -0x00020 );
  ASTAR_TEST_S18_OP(27,  addigp, 0x0fffffffffffffc0, 0x1000000000000000, -0x00040 );
  ASTAR_TEST_S18_OP(28,  addigp, 0x0fffffffffffff80, 0x1000000000000000, -0x00080 );
  ASTAR_TEST_S18_OP(29,  addigp, 0x0fffffffffffff00, 0x1000000000000000, -0x00100 );
  ASTAR_TEST_S18_OP(30,  addigp, 0x0ffffffffffffe00, 0x1000000000000000, -0x00200 );
  ASTAR_TEST_S18_OP(31,  addigp, 0x0ffffffffffffc00, 0x1000000000000000, -0x00400 );
  ASTAR_TEST_S18_OP(32,  addigp, 0x0ffffffffffff800, 0x1000000000000000, -0x00800 );
  ASTAR_TEST_S18_OP(33,  addigp, 0x0ffffffffffff000, 0x1000000000000000, -0x01000 );
  ASTAR_TEST_S18_OP(34,  addigp, 0x0fffffffffffe000, 0x1000000000000000, -0x02000 );
  ASTAR_TEST_S18_OP(35,  addigp, 0x0fffffffffffc000, 0x1000000000000000, -0x04000 );
  ASTAR_TEST_S18_OP(36,  addigp, 0x0fffffffffff8000, 0x1000000000000000, -0x08000 );
  ASTAR_TEST_S18_OP(37,  addigp, 0x0fffffffffff0000, 0x1000000000000000, -0x10000 );

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END
