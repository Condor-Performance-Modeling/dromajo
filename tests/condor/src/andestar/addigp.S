#-----------------------------------------------------------------------------
# AndeStar addigp.S
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

#                    #   instr   expect      x3          imm
  ASTAR_TEST_S18_OP( 2,  addigp, 0x00000000, 0x00000000, 0x00000 );
  ASTAR_TEST_S18_OP( 3,  addigp, 0x00000001, 0x00000000, 0x00001 );
  ASTAR_TEST_S18_OP( 4,  addigp, 0x00000002, 0x00000000, 0x00002 );
  ASTAR_TEST_S18_OP( 5,  addigp, 0x00000004, 0x00000000, 0x00004 );
  ASTAR_TEST_S18_OP( 6,  addigp, 0x00000008, 0x00000000, 0x00008 );
  ASTAR_TEST_S18_OP( 7,  addigp, 0x00000010, 0x00000000, 0x00010 );
  ASTAR_TEST_S18_OP( 8,  addigp, 0x00000020, 0x00000000, 0x00020 );
  ASTAR_TEST_S18_OP( 9,  addigp, 0x00000040, 0x00000000, 0x00040 );
  ASTAR_TEST_S18_OP(10,  addigp, 0x00000080, 0x00000000, 0x00080 );
  ASTAR_TEST_S18_OP(11,  addigp, 0x00000100, 0x00000000, 0x00100 );
  ASTAR_TEST_S18_OP(12,  addigp, 0x00000200, 0x00000000, 0x00200 );
  ASTAR_TEST_S18_OP(13,  addigp, 0x00000400, 0x00000000, 0x00400 );
  ASTAR_TEST_S18_OP(14,  addigp, 0x00000800, 0x00000000, 0x00800 );
  ASTAR_TEST_S18_OP(15,  addigp, 0x00001000, 0x00000000, 0x01000 );
  ASTAR_TEST_S18_OP(16,  addigp, 0x00002000, 0x00000000, 0x02000 );
  ASTAR_TEST_S18_OP(17,  addigp, 0x00004000, 0x00000000, 0x04000 );
  ASTAR_TEST_S18_OP(18,  addigp, 0x00008000, 0x00000000, 0x08000 );
# FIXME: add neg offsets cases
#  ASTAR_TEST_S18_OP(20,  addigp, 0xFFFFFFFF, 0x00000000, -1 );  # -1

# If it makes it this far nothing has failed, skip over <fail>:
  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END
