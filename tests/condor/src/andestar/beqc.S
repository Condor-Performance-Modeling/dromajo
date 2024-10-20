#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# beqc.S
#-----------------------------------------------------------------------------

#include "riscv_test.h"
#include "test_macros.h"
#include "andestar_test.h"
#include "andestar_test_macros.h"

RVTEST_RV64U
ASTAR_CODE_BEGIN

  # Each test checks both forward and backward branches

  #                                           val,    imm
  ASTAR_TEST_BRANCH_OP_TAKEN(       2, beqc, 0x00,   0x00);
  ASTAR_TEST_BRANCH_OP_TAKEN(       3, beqc, 0x01,   0x01);
  ASTAR_TEST_BRANCH_OP_TAKEN(       4, beqc, 0x02,   0x02);
  ASTAR_TEST_BRANCH_OP_TAKEN(       5, beqc, 0x04,   0x04);
  ASTAR_TEST_BRANCH_OP_TAKEN(       6, beqc, 0x08,   0x08);
  ASTAR_TEST_BRANCH_OP_TAKEN(       7, beqc, 0x10,   0x10);
  ASTAR_TEST_BRANCH_OP_TAKEN(       8, beqc, 0x20,   0x20);
  ASTAR_TEST_BRANCH_OP_TAKEN(       9, beqc, 0x40,   0x40);

  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   10, beqc, 0x00,   0x77);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   11, beqc, 0x01,   0x66);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   12, beqc, 0x02,   0x55);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   13, beqc, 0x04,   0x44);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   14, beqc, 0x08,   0x33);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   15, beqc, 0x10,   0x22);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   16, beqc, 0x20,   0x11);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   17, beqc, 0x40,   0x00);

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END
