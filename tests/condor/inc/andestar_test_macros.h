#pragma once

#define ASTAR_TEST_CASE( testnum, testreg, correctval, code... ) \
test_ ## testnum: \
    li  ASTAR_TESTNUM, testnum; \
    code; \
    li  x7, MASK_XLEN(correctval); \
    bne testreg, x7, astar_fail;

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
