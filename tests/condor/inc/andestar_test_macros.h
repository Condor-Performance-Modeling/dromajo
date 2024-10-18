#pragma once

#define ASTAR_TEST_CASE( testnum, testreg, correctval, code... ) \
test_ ## testnum: \
    li  ASTAR_TESTNUM, testnum; \
    code; \
    li  x7, MASK_XLEN(correctval); \
    bne testreg, x7, astar_fail;

#define ASTAR_SEXT_IMM18(x) ((x) | (-(((x) >> 17) & 1) << 17))





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
