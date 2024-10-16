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
      lui  x7,     %hi(expect);   /* upper 20 bits of expect  */ \
      addi x7, x7, %lo(expect);   /* lower 12 bits of expect  */ \
      lui  x3,     %hi(x3_value); /* upper 20 bits of initial */ \
      addi x3, x3, %lo(x3_value); /* lower 12 bits of initial */ \
      inst x14, imm18; \
    )

