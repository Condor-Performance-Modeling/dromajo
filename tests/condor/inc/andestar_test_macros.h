#pragma once

#define ASTAR_TEST_CASE( testnum, testreg, correctval, code... ) \
test_ ## testnum: \
    li  ASTAR_TESTNUM, testnum; \
    code; \
    li  x7, MASK_XLEN(correctval); \
    bne testreg, x7, astar_fail;

#define ASTAR_TEST_RR_OP( testnum, inst, expect, val1, val2 ) \
    ASTAR_TEST_CASE( testnum, x14, expect, \
      li  x11, MASK_XLEN(val1); \
      li  x12, MASK_XLEN(val2); \
      inst x14, x11, x12; \
    )

#define ASTAR_TEST_S18_OP( testnum, inst, expect, x3_value, imm18 ) \
  ASTAR_TEST_CASE( testnum, x14, expect, \
    li   x7, expect; \
    li   x3, x3_value; \
    inst x14, imm18; \
    )

#define ASTAR_TEST_LD_OP( testnum, inst, expect, offset, base ) \
    ASTAR_TEST_CASE( testnum, x14, expect, \
      li   x15, expect; /* Tell the exception handler the expected result. */ \
      la   x3, base; \
      inst x14, offset; \
    )

#define ASTAR_TEST_ST_OP( testnum, load_inst, store_inst, expect, offset, base ) \
    ASTAR_TEST_CASE( testnum, x14, expect, \
      la  x3, base; \
      li  x1, expect; \
      la  x15, 7f; /* Tell the exception handler how to skip this test. */ \
      store_inst x1, offset; \
      load_inst x14, offset; \
      j 8f; \
7:    \
      /* Set up the correct result for TEST_CASE(). */ \
      mv x14, x1; \
8:    \
    )

#define ASTAR_TEST_BFO_OP( testnum, inst, expect, val, msb, lsb ) \
    ASTAR_TEST_CASE( testnum, x14, expect, \
      li   x11, val; \
      inst x14, x11, msb, lsb; \
    )

#define ASTAR_TEST_LEA_OP( testnum, inst, expect, offset, base ) \
    ASTAR_TEST_CASE( testnum, x14, expect, \
      li  x11, base; \
      li  x12, offset; \
      inst x14, x11, x12; \
    )

#define ASTAR_TEST_BRANCH_OP_TAKEN( testnum, inst, val, imm ) \
test_ ## testnum: \
    li  ASTAR_TESTNUM, testnum; \
    li  x1, val; \
    inst x1, imm, 2f; \
    bne x0, ASTAR_TESTNUM, astar_fail; \
1:  bne x0, ASTAR_TESTNUM, 3f; \
2:  inst x1, imm, 1b; \
    bne x0, ASTAR_TESTNUM, astar_fail; \
3:

#define ASTAR_TEST_BRANCH_OP_NOTTAKEN( testnum, inst, val, imm ) \
test_ ## testnum: \
    li  ASTAR_TESTNUM, testnum; \
    li  x1, val; \
    inst x1, imm, 1f; \
    bne x0, ASTAR_TESTNUM, 2f; \
1:  bne x0, ASTAR_TESTNUM, astar_fail; \
2:  inst x1, imm, 1b; \
3:
