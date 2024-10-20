#-----------------------------------------------------------------------------
# (c) 2024 Condor Computing Corp
#-----------------------------------------------------------------------------
# bbc.S
#-----------------------------------------------------------------------------

#include "riscv_test.h"
#include "test_macros.h"
#include "andestar_test.h"
#include "andestar_test_macros.h"

RVTEST_RV64U
ASTAR_CODE_BEGIN

  # Each test checks both forward and backward branches

  #                                                     val, imm
  ASTAR_TEST_BRANCH_OP_TAKEN(    2, bbc, 0x7766554433221100,   0);
  ASTAR_TEST_BRANCH_OP_TAKEN(    3, bbc, 0x7766554433221100,  31);
  ASTAR_TEST_BRANCH_OP_TAKEN(    4, bbc, 0x7766554433221100,  32);
  ASTAR_TEST_BRANCH_OP_TAKEN(    5, bbc, 0x7766554433221100,  63);

  ASTAR_TEST_BRANCH_OP_NOTTAKEN( 6, bbc, 0x7766554433221100,   8);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN( 7, bbc, 0x7766554433221100,  29);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN( 8, bbc, 0x7766554433221100,  34);
  ASTAR_TEST_BRANCH_OP_NOTTAKEN( 9, bbc, 0x7766554433221100,  62);

  ASTAR_TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN

  TEST_DATA

RVTEST_DATA_END
