#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# bnec.S
#-----------------------------------------------------------------------------

#include "riscv_test.h"
#include "test_macros.h"
#include "andestar_test.h"
#include "andestar_test_macros.h"

RVTEST_RV64U
ASTAR_CODE_BEGIN

  # Each test checks both forward and backward branches

  #                                           val,    imm
  ASTAR_TEST_BRANCH_OP_TAKEN(       2, bnec, 0x00,   0x77);
  ASTAR_TEST_BRANCH_OP_TAKEN(       3, bnec, 0x01,   0x66);
  ASTAR_TEST_BRANCH_OP_TAKEN(       4, bnec, 0x02,   0x55);
  ASTAR_TEST_BRANCH_OP_TAKEN(       5, bnec, 0x04,   0x44);
  ASTAR_TEST_BRANCH_OP_TAKEN(       6, bnec, 0x08,   0x33);
  ASTAR_TEST_BRANCH_OP_TAKEN(       7, bnec, 0x10,   0x22);
  ASTAR_TEST_BRANCH_OP_TAKEN(       8, bnec, 0x20,   0x11);
  ASTAR_TEST_BRANCH_OP_TAKEN(       9, bnec, 0x40,   0x00);

  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   10, bnec, 0x00,   0x00);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   11, bnec, 0x01,   0x01);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   12, bnec, 0x02,   0x02);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   13, bnec, 0x04,   0x04);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   14, bnec, 0x08,   0x08);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   15, bnec, 0x10,   0x10);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   16, bnec, 0x20,   0x20);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN(   17, bnec, 0x40,   0x40);

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END
