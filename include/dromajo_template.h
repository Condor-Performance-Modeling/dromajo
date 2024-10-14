/*
 * RISCV emulator
 *
 * Copyright (c) 2016 Fabrice Bellard
 * Copyright (C) 2017,2018,2019, Esperanto Technologies Inc.
 * Copyright (C) 2023-2024, Condor Computing Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * THIS FILE IS BASED ON THE RISCVEMU SOURCE CODE WHICH IS DISTRIBUTED
 * UNDER THE FOLLOWING LICENSE:
 *
 * Copyright (c) 2016 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#if XLEN == 32
#define uintx_t uint32_t
#define intx_t  int32_t
#elif XLEN == 64
#define uintx_t uint64_t
#define intx_t  int64_t
#elif XLEN == 128
#define uintx_t uint128_t
#define intx_t  int128_t
#else
#error unsupported XLEN
#endif

#include "dromajo_stf.h"
#include <limits>

//#define EN_ZBA (s->machine->common.ext_flags.zba == true)
//#define EN_ZBB (s->machine->common.ext_flags.zbb == true)
//#define EN_ZBC (s->machine->common.ext_flags.zbc == true)
//#define EN_ZBS (s->machine->common.ext_flags.zbs == true)

#define EN_ZBA 1
#define EN_ZBB 1
#define EN_ZBC 1
#define EN_ZBS 1

//TODO: add this to extension support
#define EN_ASTAR 1
#define EN_CODENSE 0
//TODO: add this to extension support
#define _GP 0x3
//TODO: modify src to use this instead of cond. compile
extern uint32_t _XLEN;

#define HALF_WORD_MASK 0xFFFF

#define CAPTURE_LOG    1
#define REPORT_ILLEGAL 0

#if REPORT_ILLEGAL == 1
#define ILLEGAL_INSTR(S) { \
  fprintf(dromajo_stderr,"ILLEGAL INSTR %s PC:0x%lx ENC:0x%08x OPC:%03x\n",S,s->pc,insn,insn&0x7F); \
  goto illegal_insn; \
}
#else
#define ILLEGAL_INSTR(S) { \
  goto illegal_insn; \
}
#endif

#if CAPTURE_LOG == 1 
#define CAPTURED_INSTR(S) { \
  fprintf(dromajo_stderr,"CAPTURED INSTR %s PC:0x%lx ENC:0x%08x\n",S,s->pc,insn);  \
}
#else
#define CAPTURED_INSTR(S)
#endif

// =========================================================================
// =========================================================================
static inline intx_t glue(div, XLEN)(intx_t a, intx_t b) {
    if (b == 0) {
        return -1;
    } else if (a == ((intx_t)1 << (XLEN - 1)) && b == -1) {
        return a;
    } else {
        return a / b;
    }
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
static inline uintx_t glue(divu, XLEN)(uintx_t a, uintx_t b) {
    if (b == 0) {
        return -1;
    } else {
        return a / b;
    }
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
static inline intx_t glue(rem, XLEN)(intx_t a, intx_t b) {
    if (b == 0) {
        return a;
    } else if (a == ((intx_t)1 << (XLEN - 1)) && b == -1) {
        return 0;
    } else {
        return a % b;
    }
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
static inline uintx_t glue(remu, XLEN)(uintx_t a, uintx_t b) {
    if (b == 0) {
        return a;
    } else {
        return a % b;
    }
}
// =========================================================================
// =========================================================================
static inline uintx_t glue(clmul,XLEN)(uintx_t v1,uintx_t v2) { 
    assert(XLEN == 32 || XLEN == 64 || XLEN == 128);
   
    uintx_t val = 0;
    for (unsigned i = 0; i < XLEN; ++i)
        if ((v2 >> i) & 1) val ^= v1 << i;
    return val;
}
// -------------------------------------------------------------------------
static inline uintx_t glue(clmulh,XLEN)(uintx_t v1,uintx_t v2) { 
    assert(XLEN == 32 || XLEN == 64 || XLEN == 128);
  
    uintx_t val = 0;
    for (unsigned i = 1; i < XLEN; ++i)
        if ((v2 >> i) & 1) val ^= v1 >> (XLEN - i);
    return val;
}
// -------------------------------------------------------------------------
static inline uintx_t glue(clmulr,XLEN)(uintx_t v1,uintx_t v2) { 
    assert(XLEN == 32 || XLEN == 64 || XLEN == 128);
  
    uintx_t val = 0;
    for (unsigned i = 0; i < XLEN; ++i)
        if ((v2 >> i) & 1) val ^= v1 >> (XLEN - i - 1);
    return val;
}
// -------------------------------------------------------------------------
static inline uintx_t glue(cpop,XLEN)(uintx_t val) { 
    assert(XLEN == 32 || XLEN == 64 || XLEN == 128);
    
    if(XLEN == 32)      return __builtin_popcount(val);
    else if(XLEN == 64) return __builtin_popcountl(val);
    else {
        uintx_t bit_count = 0;
        while (val) {
            val &= (val - 1);
            bit_count++;
        }
        return bit_count;
    } 
}
// -------------------------------------------------------------------------
static inline uintx_t glue(cpopw,XLEN)(uintx_t val) {
    uint32_t _val = (uint32_t) val;
    return __builtin_popcount(_val);
}
// -------------------------------------------------------------------------
static inline uintx_t glue(ctz_,XLEN)(uintx_t val) {
    assert(XLEN == 32 || XLEN == 64 || XLEN == 128);
    if(val == 0) return XLEN;

    if(XLEN == 32) return __builtin_ctz(val);
    else if(XLEN == 64) return __builtin_ctzl(val);
    else {
        uint64_t lo_val = val & 0xFFFFFFFFFFFFFFFF;
        uint64_t hi_val = ((uint128_t)val >> 64);
        if(lo_val != 0) return __builtin_ctzl(lo_val);
        if(hi_val != 0) return __builtin_ctzl(hi_val);
        return 128;
    } 
}
// -------------------------------------------------------------------------
static inline uintx_t glue(ctzw,XLEN)(uintx_t val) {
    uint32_t _val = val;
    return __builtin_ctz(_val);
}
// -------------------------------------------------------------------------
static inline intx_t glue(sext_h,XLEN)(uintx_t val) { 
    intx_t tmp = val << (XLEN-16);
    return (intx_t) (((intx_t)tmp) >> (XLEN-16));
}
// -------------------------------------------------------------------------
static inline intx_t glue(sext_b,XLEN)(uintx_t val) { 
    intx_t tmp = val << (XLEN-8);
    return (intx_t) (((intx_t)tmp) >> (XLEN-8));
}
// -------------------------------------------------------------------------
static inline intx_t glue(clz,XLEN)(uintx_t val) { 
    assert(XLEN == 32 || XLEN == 64 || XLEN == 128);
    if(val == 0) return XLEN;
    else if(XLEN == 32) return __builtin_ctz(val);
    else if(XLEN == 64) return __builtin_ctzl(val);   
    else {
        for (int i = XLEN-1; i >= 0; i--) {
            if (val & ((__uint128_t)1 << i)) {
                return XLEN-1 - i;
            }
        }      
    }
}
// -------------------------------------------------------------------------
static inline intx_t glue(clzw,XLEN)(uintx_t val) { 
    if(val == 0) return 32;
    else {
      uint32_t _val = (uint32_t)val;
      return __builtin_ctz(_val);
    }
}
// -------------------------------------------------------------------------
#if XLEN >= 64
// -------------------------------------------------------------------------
static inline uint64_t clear_bits(uint64_t value, int start, int end) {
    uintx_t mask = ((1ULL << (start + 1)) - 1) & ~((1ULL << end) - 1);
    return value & ~mask;
}
// -------------------------------------------------------------------------
// FIXME: The documentation from andes isn't clear about what should happen
// when msb or lsb is zero. This code is suspect until tested 
// -------------------------------------------------------------------------
static inline uint64_t bfoz_oper(uint32_t insn,uintx_t Rs1) {

    uint64_t val=0;
    uint32_t msb = insn >> 26 & 0x3F;
    uint32_t lsb = insn >> 20 & 0x3F;

    uint32_t lsbp1 = lsb + 1;
    uint32_t msbm1 = msb - 1;
    uint32_t lsbm1 = lsb - 1;

    if (msb == 0) {
        // Set val[lsb] = Rs1[0]
        val |= ((Rs1 & 1) << lsb);

        // Zero out the upper part if lsb < 63
        if (lsb < 63) {
            val = clear_bits(val, 63, lsbp1);
        }
        
        // Zero out the lower part if lsb > 0
        if (lsb > 0) {
            val = clear_bits(val, lsbm1, 0);
        }
    } else if (msb < lsb) {
        // Case: msb < lsb
        int lenm1 = lsb - msb;
        
        // Set val[lsb:msb] = Rs1[lenm1:0]
        uint64_t mask = (1ULL << (lenm1 + 1)) - 1;
        val |= (Rs1 & mask) << msb;
        
        // Zero out the upper part if lsb < 63
        if (lsb < 63) {
            val = clear_bits(val, 63, lsbp1);
        }
  
        // Zero out lower part val[msbm1:0]
        val = clear_bits(val, msbm1, 0);
    } else {
        // Case: msb >= lsb
        int lenm1 = msb - lsb;
        
        // Set val[lenm1:0] = Rs1[msb:lsb]
        uint64_t mask = (1ULL << (lenm1 + 1)) - 1;
        val |= (Rs1 & mask) << lsb;

        // Zero out upper part val[63:(lenm1+1)]
        val = clear_bits(val, 63, lenm1 + 1);
    }

    return val;
}
// -------------------------------------------------------------------------
static inline uint64_t repeat_bit(uint64_t value, int bit_pos) {
    // Extract the bit at bit_pos and extend it across the full 64 bits
    uint64_t bit = (value >> bit_pos) & 1;
    return bit ? ~((uint64_t)0) : 0;
}
// -------------------------------------------------------------------------
// FIXME: The documentation from andes isn't clear about what should happen
// when msb or lsb is zero. This code is suspect until tested 
// -------------------------------------------------------------------------
static inline uint64_t bfos_oper(uint32_t insn,uint64_t Rs1) {

    uint64_t Rd=0;
    uint32_t MSB = insn >> 26 & 0x3F;
    uint32_t LSB = insn >> 20 & 0x3F;

    uint64_t lsbp1 = LSB + 1;
    uint64_t msbm1 = MSB - 1;
//    uint64_t lsbm1 = LSB - 1;

    uint64_t lenm1;


    if (MSB == 0) {
        // Case MSB == 0
        Rd |= ((Rs1 & 1) << LSB);  // Rd[LSB] = Rs1[0]

        if (LSB < 63) {
            // Rd[63:lsbp1] = REPEAT(Rs1[0])
            // Extend Rs1[0] to Rd[63:lsbp1]
            Rd |= repeat_bit(Rs1, 0) & (~((1ULL << (lsbp1)) - 1));
        }
        if (LSB > 0) {
            // Rd[lsbm1:0] = 0 (using lsbm1 here)
            Rd &= ~((1ULL << (LSB)) - 1);  // Clear bits from lsbm1 down to 0
        }
    } else if (MSB < LSB) {
        // Case MSB < LSB
        lenm1 = LSB - MSB;

        // Rd[LSB:MSB] = Rs1[lenm1:0]
        Rd |= (Rs1 & ((1ULL << (lenm1 + 1)) - 1)) << MSB;

        if (LSB < 63) {
            // Rd[63:lsbp1] = REPEAT(Rs1[lenm1])
            Rd |= repeat_bit(Rs1, lenm1) & (~((1ULL << (lsbp1)) - 1));
        }
        // Rd[msbm1:0] = 0 (using msbm1 here)
        Rd &= ~((1ULL << (msbm1 + 1)) - 1);
    } else {
        // Case MSB >= LSB
        lenm1 = MSB - LSB;

        // Rd[lenm1:0] = Rs1[MSB:LSB]
        Rd |= (Rs1 >> LSB) & ((1ULL << (lenm1 + 1)) - 1);

        // Rd[63:(lenm1+1)] = REPEAT(Rs1[MSB])
        Rd |= repeat_bit(Rs1, MSB) & (~((1ULL << (lenm1 + 1)) - 1));
    }

    return Rd;
}
#endif
// =========================================================================
// =========================================================================
#if XLEN == 32

static inline uint32_t mulh32(int32_t a, int32_t b) { return ((int64_t)a * (int64_t)b) >> 32; }

static inline uint32_t mulhsu32(int32_t a, uint32_t b) { return ((int64_t)a * (int64_t)b) >> 32; }

static inline uint32_t mulhu32(uint32_t a, uint32_t b) { return ((int64_t)a * (int64_t)b) >> 32; }

#elif XLEN == 64 && defined(HAVE_INT128)

static inline uint64_t mulh64(int64_t a, int64_t b) { return ((int128_t)a * (int128_t)b) >> 64; }

static inline uint64_t mulhsu64(int64_t a, uint64_t b) { return ((int128_t)a * (int128_t)b) >> 64; }

static inline uint64_t mulhu64(uint64_t a, uint64_t b) { return ((int128_t)a * (int128_t)b) >> 64; }

#else

#if XLEN == 64
#define UHALF     uint32_t
#define UHALF_LEN 32
#elif XLEN == 128
#define UHALF     uint64_t
#define UHALF_LEN 64
#else
#error unsupported XLEN
#endif

static uintx_t glue(mulhu, XLEN)(uintx_t a, uintx_t b) {
    UHALF   a0, a1, b0, b1, r2, r3;
    uintx_t r00, r01, r10, r11, c;
    a0 = a;
    a1 = a >> UHALF_LEN;
    b0 = b;
    b1 = b >> UHALF_LEN;

    r00 = (uintx_t)a0 * (uintx_t)b0;
    r01 = (uintx_t)a0 * (uintx_t)b1;
    r10 = (uintx_t)a1 * (uintx_t)b0;
    r11 = (uintx_t)a1 * (uintx_t)b1;

    //    r0 = r00;
    c = (r00 >> UHALF_LEN) + (UHALF)r01 + (UHALF)r10;
    //    r1 = c;
    c  = (c >> UHALF_LEN) + (r01 >> UHALF_LEN) + (r10 >> UHALF_LEN) + (UHALF)r11;
    r2 = c;
    r3 = (c >> UHALF_LEN) + (r11 >> UHALF_LEN);

    //    *plow = ((uintx_t)r1 << UHALF_LEN) | r0;
    return ((uintx_t)r3 << UHALF_LEN) | r2;
}

#undef UHALF

static inline uintx_t glue(mulh, XLEN)(intx_t a, intx_t b) {
    uintx_t r1;
    r1 = glue(mulhu, XLEN)(a, b);
    if (a < 0)
        r1 -= a;
    if (b < 0)
        r1 -= b;
    return r1;
}

static inline uintx_t glue(mulhsu, XLEN)(intx_t a, uintx_t b) {
    uintx_t r1;
    r1 = glue(mulhu, XLEN)(a, b);
    if (a < 0)
        r1 -= a;
    return r1;
}

#endif

#define DUP2(F, n)  F(n) F(n + 1)
#define DUP4(F, n)  DUP2(F, n) DUP2(F, n + 2)
#define DUP8(F, n)  DUP4(F, n) DUP4(F, n + 4)
#define DUP16(F, n) DUP8(F, n) DUP8(F, n + 8)
#define DUP32(F, n) DUP16(F, n) DUP16(F, n + 16)

#define C_QUADRANT(n)   \
    case n + (0 << 2):  \
    case n + (1 << 2):  \
    case n + (2 << 2):  \
    case n + (3 << 2):  \
    case n + (4 << 2):  \
    case n + (5 << 2):  \
    case n + (6 << 2):  \
    case n + (7 << 2):  \
    case n + (8 << 2):  \
    case n + (9 << 2):  \
    case n + (10 << 2): \
    case n + (11 << 2): \
    case n + (12 << 2): \
    case n + (13 << 2): \
    case n + (14 << 2): \
    case n + (15 << 2): \
    case n + (16 << 2): \
    case n + (17 << 2): \
    case n + (18 << 2): \
    case n + (19 << 2): \
    case n + (20 << 2): \
    case n + (21 << 2): \
    case n + (22 << 2): \
    case n + (23 << 2): \
    case n + (24 << 2): \
    case n + (25 << 2): \
    case n + (26 << 2): \
    case n + (27 << 2): \
    case n + (28 << 2): \
    case n + (29 << 2): \
    case n + (30 << 2): \
    case n + (31 << 2):

#define GET_PC() (target_ulong)((uintptr_t)code_ptr + code_to_pc_addend)
#define GET_INSN_COUNTER() (insn_counter_addend - n_cycles)

#define C_NEXT_INSN \
    code_ptr += 2;  \
    break
#define NEXT_INSN  \
    code_ptr += 4; \
    break
#define JUMP_INSN(kind)            \
    do {                           \
        code_ptr          = NULL;  \
        code_end          = NULL;  \
        code_to_pc_addend = s->pc; \
        s->info           = kind;  \
        s->next_addr      = s->pc; \
        goto jump_insn;            \
    } while (0)

#define chkfp32 glue(chkfp32, XLEN)

static uint32_t chkfp32(target_ulong a) {
    if ((a & 0xFFFFFFFF00000000ULL) != 0xFFFFFFFF00000000ULL)
        return -1U << 22;  // Not boxed => return float32 QNAN

    return (uint32_t)a;
}

/*
 * Table 2.1: Return-address stack prediction hints
 *      rd      rs1     rs1=rd          RAS action
 *    !x1/x5  !x1/x5       -            none
 *    !x1/x5   x1/x5       -            pop
 *     x1/x5  !x1/x5       -            push
 *     x1/x5   x1/x5       0            pop, then push
 *     x1/x5   x1/x5       1            push
 */

int no_inline glue(riscv_cpu_interp, XLEN)(RISCVCPUState *s, int n_cycles);

int no_inline glue(riscv_cpu_interp, XLEN)(RISCVCPUState *s, int n_cycles) {
    uint32_t     opcode, insn, rd, rs1, rs2, funct2, funct3;
    uint32_t     _funct3, _funct6, _funct7, _funct12, _shamt;
    int32_t      imm, cond, err;
    target_ulong addr, val, val2;
    uint8_t *    code_ptr, *code_end;
    target_ulong code_to_pc_addend;
    uint64_t     insn_counter_addend;
    uint64_t     insn_counter_start = s->insn_counter;

    //Added for A*
    bool isBranchGroup=false;
    target_ulong _new_pc;
    uint64_t     simm11,cimm7,cimm6,zero32_rs2;
    uint32_t     _bit30;
    (void) simm11; //GCC complains for XLEN != 64
    (void) cimm7; 
    (void) cimm6; 
    (void) _bit30; 
    (void) zero32_rs2;
    (void) _new_pc;
    (void) isBranchGroup;

#if FLEN > 0
    uint32_t rs3;
    int32_t  rm;
#endif
#if VLEN > 0
    target_ulong avl, vl;
    uint8_t      vset, vmem_result, funct6;
    bool         vm;
    clear_most_recently_written_vregs(s);
#endif
    int insn_executed               = 0;
    s->most_recently_written_reg    = -1;
    s->most_recently_written_fp_reg = -1;
    s->info                         = ctf_nop;

    if (n_cycles == 0)
        return 0;
    insn_counter_addend = s->insn_counter + n_cycles;

    /* check pending interrupts */
    if (unlikely(((s->mip & s->mie) != 0) && (s->machine->common.pending_interrupt != -1 || !s->machine->common.cosim))) {
        if (raise_interrupt(s)) {
            --insn_counter_addend;
            goto done_interp;
        }
    }

    s->pending_exception = -1;
    s->last_data_vaddr = std::numeric_limits<decltype(s->last_data_vaddr)>::max();
    n_cycles++;
    /* Note: we assume NULL is represented as a zero number */
    code_ptr          = NULL;
    code_end          = NULL;
    code_to_pc_addend = s->pc;
    /* we use a single execution loop to keep a simple control flow
       for emscripten */
    for (;;) {
        s->last_pc = s->pc;
        s->pc = GET_PC();
        if (unlikely(!--n_cycles))
            goto the_end;

        ++insn_executed;

        if (check_triggers(s, MCONTROL_EXECUTE, s->pc))
            goto exception;

        if (unlikely(code_ptr >= code_end)) {
            uint32_t     tlb_idx;
            uint16_t     insn_high;
            target_ulong addr;

            /* check pending interrupts */
            if (unlikely(((s->mip & s->mie) != 0) && (s->machine->common.pending_interrupt != -1 || !s->machine->common.cosim))) {
                if (raise_interrupt(s)) {
                    goto the_end;
                }
            }

            addr    = s->pc;
            tlb_idx = (addr >> PG_SHIFT) & (TLB_SIZE - 1);
            if (likely(s->tlb_code[tlb_idx].vaddr == (addr & ~PG_MASK))) {
                /* TLB match */
                uintptr_t mem_addend;
                mem_addend        = s->tlb_code[tlb_idx].mem_addend;
                code_ptr          = (uint8_t *)(mem_addend + (uintptr_t)addr);
                code_end          = (uint8_t *)(mem_addend + (uintptr_t)((addr & ~PG_MASK) + PG_MASK - 1));
                code_to_pc_addend = addr - (uintptr_t)code_ptr;
                if (unlikely(code_ptr >= code_end)) {
                    /* instruction is potentially half way between two
                       pages ? */
                    insn = *(uint16_t *)code_ptr;
                    if ((insn & 3) == 3) {
                        /* instruction is half way between two pages */
                        if (unlikely(target_read_insn_u16(s, &insn_high, addr + 2)))
                            goto mmu_exception;
                        insn |= insn_high << 16;
                    }
                } else {
                    insn = get_insn32(code_ptr);
                }

            } else {
                if (unlikely(target_read_insn_slow(s, &insn, 32, addr)))
                    goto mmu_exception;
            }
        } else {
            /* fast path */
            insn = get_insn32(code_ptr);
        }

        opcode = insn & 0x7f;

        rd     = (insn >> 7) & 0x1f;
        rs1    = (insn >> 15) & 0x1f;
        rs2    = (insn >> 20) & 0x1f;
        uint8_t _rval;
        switch (opcode) {

            case 0x0b: /* Andes CUSTOM-O */
                funct2 = (insn >> 12) & 3;
                imm = sext( (get_field1(insn,31,17,17)
                           | get_field1(insn,15,15,16)
                           | get_field1(insn,17,12,14)
                           | get_field1(insn,20,11,11)
                           | get_field1(insn,21, 1,10) 
                           | get_field1(insn,14, 0, 0)),17);

                switch(funct2) {
                   case 0x00: CAPTURED_INSTR("A* LBGP");
                              addr = read_reg(_GP) + imm;
                              if (target_read_u8(s, &_rval, addr)) goto mmu_exception;
                              val = (int8_t) _rval;
                              break;
                   case 0x01: CAPTURED_INSTR("A* ADDIGP");
                              val = ((intx_t) read_reg(_GP)) + imm;
                              break;
                   case 0x02: CAPTURED_INSTR("A* LBUGP"); 
                              addr = read_reg(_GP) + imm;
                              if (target_read_u8(s, &_rval, addr)) goto mmu_exception;
                              val = _rval;
                              break;
                   case 0x03: CAPTURED_INSTR("A* SBGP"); 
                              addr = read_reg(_GP) + imm;
                              if (target_write_u8(s, addr, read_reg(rs2))) goto mmu_exception;
                              val = _rval;
                              break;
                   default:
                              ILLEGAL_INSTR("0xb-a")
                }

                if (rd != 0) write_reg(rd, val);
                NEXT_INSN;

            case 0x2b: /* Andes CUSTOM-1 */
                funct3 = (insn >> 12) & 7;
                funct2 = (insn >> 12) & 3;
                //TODO: see if this can be changed to switch, re: u/intxx_t rval scope issues.

                if(funct3 == 0x0) { // SHGP
                  CAPTURED_INSTR("A* SHGP");
                  imm = sext((get_field1(insn,31,17,17)
                            | get_field1(insn,15,15,16)
                            | get_field1(insn,17,12,14)
                            | get_field1(insn,7, 11,11)
                            | get_field1(insn,25, 5,10)
                            | get_field1(insn,8,  1,4)),17); 

                  addr = read_reg(_GP) + imm;
                  if (target_write_u16(s, addr, read_reg(rs2))) goto mmu_exception;

                } else if(funct3 == 0x1) { // LHGP
                  CAPTURED_INSTR("A* LHGP");
                  imm = sext((get_field1(insn,31,17,17)
                            | get_field1(insn,15,15,16)
                            | get_field1(insn,17,12,14)
                            | get_field1(insn,20,11,11)
                            | get_field1(insn,21, 1,10)),17); 
                  addr = read_reg(_GP) + imm;
                  uint16_t rval;
                  if (target_read_u16(s, &rval, addr)) goto mmu_exception;
                  val = (int16_t) rval;

                } else if(funct3 == 0x2) { // LWGP
                  CAPTURED_INSTR("A* LWGP");
                  imm = sext((get_field1(insn,31,18,18)
                            | get_field1(insn,21,17,17)
                            | get_field1(insn,15,15,16)
                            | get_field1(insn,17,12,14)
                            | get_field1(insn,20,11,11)
                            | get_field1(insn,22, 2,10)),18); 
                  addr = read_reg(_GP) + imm;
                  uint32_t rval;
                  if (target_read_u32(s, &rval, addr)) goto mmu_exception;
                  val = (int32_t) rval;

                } else if(funct3 == 0x3) { // LDGP
                  CAPTURED_INSTR("A* LDGP");
                  imm = sext((get_field1(insn,31,19,19)
                            | get_field1(insn,21,17,18)
                            | get_field1(insn,15,15,16)
                            | get_field1(insn,17,12,14)
                            | get_field1(insn,20,11,11)
                            | get_field1(insn,23, 3,10)),19); 
                  addr = read_reg(_GP) + imm;
                  uint64_t rval;
                  if (target_read_u64(s, &rval, addr)) goto mmu_exception;
                  val = (int64_t) rval;

                } else if(funct3 == 0x4) { // SWGP
                  CAPTURED_INSTR("A* SWGP");
                  imm = sext((get_field1(insn,31,19,19)
                            | get_field1(insn,21,17,18)
                            | get_field1(insn,15,15,16)
                            | get_field1(insn,17,12,14)
                            | get_field1(insn,20,11,11)
                            | get_field1(insn,23, 3,10)),19); 

                  addr = read_reg(_GP) + imm;
                  if (target_write_u32(s, addr, read_reg(rs2))) goto mmu_exception;

                } else if(funct3 == 0x5) { // LHUGP
                  CAPTURED_INSTR("A* LHUGP");
                  imm = sext((get_field1(insn,31,17,17)
                            | get_field1(insn,15,15,16)
                            | get_field1(insn,17,12,14)
                            | get_field1(insn,20,11,11)
                            | get_field1(insn,21, 1,10)),17); 
                  addr = read_reg(_GP) + imm;
                  uint16_t rval;
                  if (target_read_u16(s, &rval, addr)) goto mmu_exception;
                  val = rval;

                } else if(funct3 == 0x6) { // LWUGP
                  CAPTURED_INSTR("A* LWUGP");
                              //word addr by construction
                  imm = sext((get_field1(insn,31,18,18)
                            | get_field1(insn,21,17,17)
                            | get_field1(insn,15,15,16)
                            | get_field1(insn,17,12,14)
                            | get_field1(insn,20,11,11)
                            | get_field1(insn,22, 2,10)),18); 
                  addr = read_reg(_GP) + imm;
                  uint32_t rval;
                  if (target_read_u32(s, &rval, addr)) goto mmu_exception;
                  val = rval;

                } else if(funct3 == 0x7) { //SDGP
#if XLEN >= 64
                  CAPTURED_INSTR("A* SDGP");
                  //dword addr by construction
                  imm = sext((get_field1(insn,31,19,19) //31    -> 19
                            | get_field1(insn, 8,17,18) //9:8   -> 18:17
                            | get_field1(insn,15,15,16) //16:15 -> 16:15
                            | get_field1(insn,17,12,14) //19:17 -> 14:12
                            | get_field1(insn, 7,11,11) //7     -> 11
                            | get_field1(insn,25, 5,10) //30:25 -> 10:5
                            | get_field1(insn,10, 3, 4)),19); //11:10 -> 4:3
                  addr = read_reg(_GP) + imm;
                  if (target_write_u64(s, addr, read_reg(rs2))) {
                    goto mmu_exception;
                  }
                  break;
#else
                  ILLEGAL_INSTR("02b-0")
#endif
                } else if(funct3 == 0x4) { // SWGP
                  CAPTURED_INSTR("A* SWGP");
                  imm = sext((get_field1(insn,31,18,18) //31    -> 18
                            | get_field1(insn, 8,17,17) //8     -> 17
                            | get_field1(insn,15,15,16) //16:15 -> 16:15
                            | get_field1(insn,17,12,14) //19:17 -> 14:12
                            | get_field1(insn, 7,11,11) //7     -> 11
                            | get_field1(insn,25, 5,10) //30:25 -> 10:5
                            | get_field1(insn, 9, 3, 4)),18);
                  addr = read_reg(_GP) + imm;
                  if (target_write_u32(s, addr, read_reg(rs2))) {
                    goto mmu_exception;
                  }
                  break;

                } else {
                  ILLEGAL_INSTR("02b-1")
                }
                if (rd != 0) write_reg(rd, val);
                NEXT_INSN;

            // --------------------------------------------------------------
            C_QUADRANT(0)
            funct3 = (insn >> 13) & 7;
            rd     = ((insn >> 2) & 7) | 8;
            switch (funct3) {
                case 0: /* c.addi4spn */
                    imm = get_field1(insn, 11, 4, 5) | get_field1(insn, 7, 6, 9) | get_field1(insn, 6, 2, 2)
                          | get_field1(insn, 5, 3, 3);
                    if (imm == 0)
                        ILLEGAL_INSTR("000")
                    write_reg(rd, (intx_t)(read_reg(2) + imm));
                    break;
#if XLEN >= 128
                case 1: /* c.lq */
                    imm  = get_field1(insn, 11, 4, 5) | get_field1(insn, 10, 8, 8) | get_field1(insn, 5, 6, 7);
                    rs1  = ((insn >> 7) & 7) | 8;
                    addr = (intx_t)(read_reg(rs1) + imm);
                    s->last_addr = addr;
                    if (target_read_u128(s, &val, addr))
                        goto mmu_exception;
                    write_reg(rd, val);
                    break;
#elif FLEN >= 64
                case 1: /* c.fld */
                {
                    uint64_t rval;
                    if (s->fs == 0)
                        ILLEGAL_INSTR("001")
                    imm  = get_field1(insn, 10, 3, 5) | get_field1(insn, 5, 6, 7);
                    rs1  = ((insn >> 7) & 7) | 8;
                    addr = (intx_t)(read_reg(rs1) + imm);
                    if (target_read_u64(s, &rval, addr))
                        goto mmu_exception;
                    write_fp_reg(rd, rval | F64_HIGH);
                } break;
#endif
                case 2: /* c.lw */
                {
                    uint32_t rval;
                    imm  = get_field1(insn, 10, 3, 5) | get_field1(insn, 6, 2, 2) | get_field1(insn, 5, 6, 6);
                    rs1  = ((insn >> 7) & 7) | 8;
                    addr = (intx_t)(read_reg(rs1) + imm);
                    if (target_read_u32(s, &rval, addr))
                        goto mmu_exception;
                    write_reg(rd, (int32_t)rval);
                } break;
#if XLEN >= 64
                case 3: /* c.ld */
                {
                    uint64_t rval;
                    imm  = get_field1(insn, 10, 3, 5) | get_field1(insn, 5, 6, 7);
                    rs1  = ((insn >> 7) & 7) | 8;
                    addr = (intx_t)(read_reg(rs1) + imm);
                    if (target_read_u64(s, &rval, addr))
                        goto mmu_exception;
                    write_reg(rd, (int64_t)rval);
                } break;
#elif FLEN >= 32
                case 3: /* c.flw */
                {
                    uint32_t rval;
                    if (s->fs == 0)
                        ILLEGAL_INSTR("002")
                    imm  = get_field1(insn, 10, 3, 5) | get_field1(insn, 6, 2, 2) | get_field1(insn, 5, 6, 6);
                    rs1  = ((insn >> 7) & 7) | 8;
                    addr = (intx_t)(read_reg(rs1) + imm);
                    if (target_read_u32(s, &rval, addr))
                        goto mmu_exception;
                    write_fp_reg(rd, rval | F32_HIGH);
                } break;
#endif
#if XLEN >= 128
                case 5: /* c.sq */
                    imm  = get_field1(insn, 11, 4, 5) | get_field1(insn, 10, 8, 8) | get_field1(insn, 5, 6, 7);
                    rs1  = ((insn >> 7) & 7) | 8;
                    addr = (intx_t)(read_reg(rs1) + imm);
                    val  = read_reg(rd);
                    if (target_write_u128(s, addr, val))
                        goto mmu_exception;
                    break;
#elif FLEN >= 64
                case 5: /* c.fsd */
                    if (s->fs == 0)
                        ILLEGAL_INSTR("003")
                    imm  = get_field1(insn, 10, 3, 5) | get_field1(insn, 5, 6, 7);
                    rs1  = ((insn >> 7) & 7) | 8;
                    addr = (intx_t)(read_reg(rs1) + imm);
                    if (target_write_u64(s, addr, read_fp_reg(rd)))
                        goto mmu_exception;
                    break;
#endif
                case 6: /* c.sw */
                    imm  = get_field1(insn, 10, 3, 5) | get_field1(insn, 6, 2, 2) | get_field1(insn, 5, 6, 6);
                    rs1  = ((insn >> 7) & 7) | 8;
                    addr = (intx_t)(read_reg(rs1) + imm);
                    val  = read_reg(rd);
                    if (target_write_u32(s, addr, val))
                        goto mmu_exception;
                    break;
#if XLEN >= 64
                case 7: /* c.sd */
                    imm  = get_field1(insn, 10, 3, 5) | get_field1(insn, 5, 6, 7);
                    rs1  = ((insn >> 7) & 7) | 8;
                    addr = (intx_t)(read_reg(rs1) + imm);
                    val  = read_reg(rd);
                    if (target_write_u64(s, addr, val))
                        goto mmu_exception;
                    break;
#elif FLEN >= 32
                case 7: /* c.fsw */
                    if (s->fs == 0)
                        ILLEGAL_INSTR("004")
                    imm  = get_field1(insn, 10, 3, 5) | get_field1(insn, 6, 2, 2) | get_field1(insn, 5, 6, 6);
                    rs1  = ((insn >> 7) & 7) | 8;
                    addr = (intx_t)(read_reg(rs1) + imm);
                    if (target_write_u32(s, addr, read_fp_reg(rd)))
                        goto mmu_exception;
                    break;
#endif
                default: ILLEGAL_INSTR("005")
            }
            C_NEXT_INSN;
            C_QUADRANT(1)
            funct3 = (insn >> 13) & 7;
            switch (funct3) {
                case 0: /* c.addi/c.nop */
                    if (rd != 0) {
                        imm = sext(get_field1(insn, 12, 5, 5) | get_field1(insn, 2, 0, 4), 6);
                        write_reg(rd, (intx_t)(read_reg(rd) + imm));
                    }
                    break;
#if XLEN == 32
                case 1: /* c.jal */
                    imm = sext(get_field1(insn, 12, 11, 11) | get_field1(insn, 11, 4, 4) | get_field1(insn, 9, 8, 9)
                                   | get_field1(insn, 8, 10, 10) | get_field1(insn, 7, 6, 6) | get_field1(insn, 6, 7, 7)
                                   | get_field1(insn, 3, 1, 3) | get_field1(insn, 2, 5, 5),
                               12);
                    write_reg(1, GET_PC() + 2);
                    s->pc = (intx_t)(GET_PC() + imm);
                    JUMP_INSN(ctf_taken_jump);
#else
                case 1: /* c.addiw */
                    if (rd == 0)
                        ILLEGAL_INSTR("006")
                    imm = sext(get_field1(insn, 12, 5, 5) | get_field1(insn, 2, 0, 4), 6);
                    write_reg(rd, (int32_t)(read_reg(rd) + imm));
                    break;
#endif
                case 2: /* c.li */
                    if (rd != 0) {
                        imm = sext(get_field1(insn, 12, 5, 5) | get_field1(insn, 2, 0, 4), 6);
                        write_reg(rd, imm);
                    }
                    break;
                case 3:
                    if (rd == 2) {
                        /* c.addi16sp */
                        imm = sext(get_field1(insn, 12, 9, 9) | get_field1(insn, 6, 4, 4) | get_field1(insn, 5, 6, 6)
                                       | get_field1(insn, 3, 7, 8) | get_field1(insn, 2, 5, 5),
                                   10);
                        if (imm == 0)
                            ILLEGAL_INSTR("007")
                        write_reg(2, (intx_t)(read_reg(2) + imm));
                    } else if (rd != 0) {
                        /* c.lui */
                        imm = sext(get_field1(insn, 12, 17, 17) | get_field1(insn, 2, 12, 16), 18);
                        if (imm == 0)
                            ILLEGAL_INSTR("008")
                        write_reg(rd, imm);
                    } else if (sext(get_field1(insn, 12, 17, 17) | get_field1(insn, 2, 12, 16), 18) == 0)
                        ILLEGAL_INSTR("009")
                    break;
                case 4:
                    funct3 = (insn >> 10) & 3;
                    rd     = ((insn >> 7) & 7) | 8;
                    switch (funct3) {
                        case 0: /* c.srli */
                        case 1: /* c.srai */ imm = get_field1(insn, 12, 5, 5) | get_field1(insn, 2, 0, 4);
#if XLEN == 32
                            if (imm & 0x20)
                                ILLEGAL_INSTR("010")
#elif XLEN == 128
                            if (imm == 0)
                                imm = 64;
                            else if (imm >= 32)
                                imm = 128 - imm;
#endif
                            if (funct3 == 0)
                                write_reg(rd, (intx_t)((uintx_t)read_reg(rd) >> imm));
                            else
                                write_reg(rd, (intx_t)read_reg(rd) >> imm);

                            break;
                        case 2: /* c.andi */
                            imm = sext(get_field1(insn, 12, 5, 5) | get_field1(insn, 2, 0, 4), 6);
                            write_reg(rd, read_reg(rd) & imm);
                            break;
                        case 3:
                            rs2    = ((insn >> 2) & 7) | 8;
                            funct3 = ((insn >> 5) & 3) | ((insn >> (12 - 2)) & 4);
                            switch (funct3) {
                                case 0: /* c.sub */ write_reg(rd, (intx_t)(read_reg(rd) - read_reg(rs2))); break;
                                case 1: /* c.xor */ write_reg(rd, read_reg(rd) ^ read_reg(rs2)); break;
                                case 2: /* c.or */ write_reg(rd, read_reg(rd) | read_reg(rs2)); break;
                                case 3: /* c.and */ write_reg(rd, read_reg(rd) & read_reg(rs2)); break;
#if XLEN >= 64
                                case 4: /* c.subw */ write_reg(rd, (int32_t)(read_reg(rd) - read_reg(rs2))); break;
                                case 5: /* c.addw */ write_reg(rd, (int32_t)(read_reg(rd) + read_reg(rs2))); break;
#endif
                                default: ILLEGAL_INSTR("011")
                            }
                            break;
                    }
                    break;
                case 5: /* c.j */
                    imm   = sext(get_field1(insn, 12, 11, 11) | get_field1(insn, 11, 4, 4) | get_field1(insn, 9, 8, 9)
                                   | get_field1(insn, 8, 10, 10) | get_field1(insn, 7, 6, 6) | get_field1(insn, 6, 7, 7)
                                   | get_field1(insn, 3, 1, 3) | get_field1(insn, 2, 5, 5),
                               12);
                    s->pc = (intx_t)(GET_PC() + imm);
                    JUMP_INSN(ctf_taken_jump);
                case 6: /* c.beqz */
                    rs1 = ((insn >> 7) & 7) | 8;
                    imm = sext(get_field1(insn, 12, 8, 8) | get_field1(insn, 10, 3, 4) | get_field1(insn, 5, 6, 7)
                                   | get_field1(insn, 3, 1, 2) | get_field1(insn, 2, 5, 5),
                               9);
                    if (read_reg(rs1) == 0) {
                        s->pc = (intx_t)(GET_PC() + imm);
                        JUMP_INSN(ctf_taken_branch);
                    }
                    break;
                case 7: /* c.bnez */
                    rs1 = ((insn >> 7) & 7) | 8;
                    imm = sext(get_field1(insn, 12, 8, 8) | get_field1(insn, 10, 3, 4) | get_field1(insn, 5, 6, 7)
                                   | get_field1(insn, 3, 1, 2) | get_field1(insn, 2, 5, 5),
                               9);
                    if (read_reg(rs1) != 0) {
                        s->pc = (intx_t)(GET_PC() + imm);
                        JUMP_INSN(ctf_taken_branch);
                    }
                    break;
                default: ILLEGAL_INSTR("012")
            }
            C_NEXT_INSN;
            C_QUADRANT(2)
            funct3 = (insn >> 13) & 7;
            rs2    = (insn >> 2) & 0x1f;
            switch (funct3) {
                case 0: /* c.slli */ imm = get_field1(insn, 12, 5, 5) | rs2;
#if XLEN == 32
                    if (imm & 0x20)
                        ILLEGAL_INSTR("013")
#elif XLEN == 128
                    if (imm == 0)
                        imm = 64;
#endif
                    if (rd != 0)
                        write_reg(rd, (intx_t)(read_reg(rd) << imm));
                    break;
#if XLEN == 128
                case 1: /* c.lqsp */
                    if (rd == 0)
                        ILLEGAL_INSTR("014")
                    imm  = get_field1(insn, 12, 5, 5) | (rs2 & (1 << 4)) | get_field1(insn, 2, 6, 9);
                    addr = (intx_t)(read_reg(2) + imm);
                    if (target_read_u128(s, &val, addr))
                        goto mmu_exception;
                    if (rd != 0)
                        write_reg(rd, val);
                    break;
#elif FLEN >= 64
                case 1: /* c.fldsp */
                {
                    uint64_t rval;
                    if (s->fs == 0)
                        ILLEGAL_INSTR("015")
                    imm  = get_field1(insn, 12, 5, 5) | (rs2 & (3 << 3)) | get_field1(insn, 2, 6, 8);
                    addr = (intx_t)(read_reg(2) + imm);
                    if (target_read_u64(s, &rval, addr))
                        goto mmu_exception;
                    write_fp_reg(rd, rval | F64_HIGH);
                } break;
#endif
                case 2: /* c.lwsp */
                {
                    uint32_t rval;
                    if (rd == 0)
                        ILLEGAL_INSTR("016")
                    imm  = get_field1(insn, 12, 5, 5) | (rs2 & (7 << 2)) | get_field1(insn, 2, 6, 7);
                    addr = (intx_t)(read_reg(2) + imm);
                    if (target_read_u32(s, &rval, addr))
                        goto mmu_exception;
                    write_reg(rd, (int32_t)rval);
                } break;
#if XLEN >= 64
                case 3: /* c.ldsp */
                {
                    uint64_t rval;
                    if (rd == 0)
                        ILLEGAL_INSTR("017")
                    imm  = get_field1(insn, 12, 5, 5) | (rs2 & (3 << 3)) | get_field1(insn, 2, 6, 8);
                    addr = (intx_t)(read_reg(2) + imm);
                    if (target_read_u64(s, &rval, addr))
                        goto mmu_exception;
                    write_reg(rd, (int64_t)rval);
                } break;
#elif FLEN >= 32
                case 3: /* c.flwsp */
                {
                    uint32_t rval;
                    if (s->fs == 0)
                        ILLEGAL_INSTR("018")
                    imm  = get_field1(insn, 12, 5, 5) | (rs2 & (7 << 2)) | get_field1(insn, 2, 6, 7);
                    addr = (intx_t)(read_reg(2) + imm);
                    if (target_read_u32(s, &rval, addr))
                        goto mmu_exception;
                    write_fp_reg(rd, rval | F32_HIGH);
                } break;
#endif
                case 4:
                    if (((insn >> 12) & 1) == 0) {
                        if (rs2 == 0) {
                            /* c.jr */
                            if (rd == 0)
                                ILLEGAL_INSTR("019")
                            s->pc = read_reg(rd) & ~1;
                            JUMP_INSN(ctf_compute_hint(0, rd));
                        } else {
                            /* c.mv */
                            if (rd != 0)
                                write_reg(rd, read_reg(rs2));
                        }
                    } else {
                        if (rs2 == 0) {
                            if (rd == 0) {
                                /* c.ebreak */
                                s->pending_exception = CAUSE_BREAKPOINT;
                                s->pending_tval      = 0;
                                goto exception;
                            } else {
                                /* c.jalr */
                                val   = GET_PC() + 2;
                                s->pc = read_reg(rd) & ~1;
                                write_reg(1, val);
                                JUMP_INSN(ctf_compute_hint(1, rd));
                            }
                        } else {
                            if (rd != 0)
                                write_reg(rd, (intx_t)(read_reg(rd) + read_reg(rs2)));
                        }
                    }
                    break;
#if XLEN == 128
                case 5: /* c.sqsp */
                    imm  = get_field1(insn, 10, 3, 5) | get_field1(insn, 7, 6, 8);
                    addr = (intx_t)(read_reg(2) + imm);
                    if (target_write_u128(s, addr, read_reg(rs2)))
                        goto mmu_exception;
                    break;
#elif FLEN >= 64
                case 5: /* c.fsdsp */
                    if (s->fs == 0)
                        ILLEGAL_INSTR("020")
                    imm  = get_field1(insn, 10, 3, 5) | get_field1(insn, 7, 6, 8);
                    addr = (intx_t)(read_reg(2) + imm);
                    if (target_write_u64(s, addr, read_fp_reg(rs2)))
                        goto mmu_exception;
                    break;
#endif
                case 6: /* c.swsp */
                    imm  = get_field1(insn, 9, 2, 5) | get_field1(insn, 7, 6, 7);
                    addr = (intx_t)(read_reg(2) + imm);
                    if (target_write_u32(s, addr, read_reg(rs2)))
                        goto mmu_exception;
                    break;
#if XLEN >= 64
                case 7: /* c.sdsp */
                    imm  = get_field1(insn, 10, 3, 5) | get_field1(insn, 7, 6, 8);
                    addr = (intx_t)(read_reg(2) + imm);
                    if (target_write_u64(s, addr, read_reg(rs2)))
                        goto mmu_exception;
                    break;
#elif FLEN >= 32
                case 7: /* c.swsp */
                    if (s->fs == 0)
                        ILLEGAL_INSTR("021")
                    imm  = get_field1(insn, 9, 2, 5) | get_field1(insn, 7, 6, 7);
                    addr = (intx_t)(read_reg(2) + imm);
                    if (target_write_u32(s, addr, read_fp_reg(rs2)))
                        goto mmu_exception;
                    break;
#endif
                default: ILLEGAL_INSTR("022")
            }
            C_NEXT_INSN;

            case 0x37: /* lui */
                if (rd != 0)
                    write_reg(rd, (int32_t)(insn & 0xfffff000));
                NEXT_INSN;
            case 0x17: /* auipc */
                if (rd != 0)
                    write_reg(rd, (intx_t)(GET_PC() + (int32_t)(insn & 0xfffff000)));
                NEXT_INSN;
            case 0x6f: /* jal */
                imm = ((insn >> (31 - 20)) & (1 << 20)) | ((insn >> (21 - 1)) & 0x7fe) | ((insn >> (20 - 11)) & (1 << 11))
                      | (insn & 0xff000);
                imm = (imm << 11) >> 11;
                {
                    intx_t new_pc = (intx_t)(GET_PC() + imm);
                    if (!(s->misa & MCPUID_C) && (new_pc & 3) != 0) {
                        s->pending_exception = CAUSE_MISALIGNED_FETCH;
                        s->pending_tval      = 0;
                        goto exception;
                    }
                }
                if (rd != 0)
                    write_reg(rd, GET_PC() + 4);
                s->pc = (intx_t)(GET_PC() + imm);
                JUMP_INSN(ctf_taken_jump);
            case 0x67: /* jalr */
                funct3 = (insn >> 12) & 7;
                if (funct3 != 0)
                    ILLEGAL_INSTR("023")
                imm = (int32_t)insn >> 20;
                val = GET_PC() + 4;
                {
                    intx_t new_pc = (intx_t)(read_reg(rs1) + imm) & ~1;
                    if (!(s->misa & MCPUID_C) && (new_pc & 3) != 0) {
                        s->pending_exception = CAUSE_MISALIGNED_FETCH;
                        s->pending_tval      = 0;
                        goto exception;
                    }
                }
                s->pc = (intx_t)(read_reg(rs1) + imm) & ~1;
                if (rd != 0)
                    write_reg(rd, val);
                JUMP_INSN(ctf_compute_hint(rd, rs1));
            case 0x63:
                funct3 = (insn >> 12) & 7;
                switch (funct3 >> 1) {
                    case 0: /* beq/bne */ cond = (read_reg(rs1) == read_reg(rs2)); break;
                    case 2: /* blt/bge */ cond = ((target_long)read_reg(rs1) < (target_long)read_reg(rs2)); break;
                    case 3: /* bltu/bgeu */ cond = (read_reg(rs1) < read_reg(rs2)); break;
                    default: ILLEGAL_INSTR("024")
                }
                cond ^= (funct3 & 1);
                if (cond) {
                    imm = ((insn >> (31 - 12)) & (1 << 12)) | ((insn >> (25 - 5)) & 0x7e0) | ((insn >> (8 - 1)) & 0x1e)
                          | ((insn << (11 - 7)) & (1 << 11));
                    imm = (imm << 19) >> 19;

                    intx_t new_pc = (intx_t)(GET_PC() + imm);
                    if (!(s->misa & MCPUID_C) && (new_pc & 3) != 0) {
                        s->pending_exception = CAUSE_MISALIGNED_FETCH;
                        s->pending_tval      = 0;
                        goto exception;
                    }

                    s->pc = (intx_t)(GET_PC() + imm);
                    JUMP_INSN(ctf_taken_branch);
                }
                NEXT_INSN;
            case 0x03: /* load */
                funct3 = (insn >> 12) & 7;
                imm    = (int32_t)insn >> 20;
                addr   = read_reg(rs1) + imm;

                switch (funct3) {
                    case 0: /* lb */
                    {
                        uint8_t rval;
                        if (target_read_u8(s, &rval, addr))
                            goto mmu_exception;
                        val = (int8_t)rval;
                    } break;
                    case 1: /* lh */
                    {
                        uint16_t rval;
                        if (target_read_u16(s, &rval, addr))
                            goto mmu_exception;
                        val = (int16_t)rval;
                    } break;
                    case 2: /* lw */
                    {
                        uint32_t rval;
                        if (target_read_u32(s, &rval, addr))
                            goto mmu_exception;
                        val = (int32_t)rval;
                    } break;
                    case 4: /* lbu */
                    {
                        uint8_t rval;
                        if (target_read_u8(s, &rval, addr))
                            goto mmu_exception;
                        val = rval;
                    } break;
                    case 5: /* lhu */
                    {
                        uint16_t rval;
                        if (target_read_u16(s, &rval, addr))
                            goto mmu_exception;
                        val = rval;
                    } break;
#if XLEN >= 64
                    case 3: /* ld */
                    {
                        uint64_t rval;
                        if (target_read_u64(s, &rval, addr))
                            goto mmu_exception;
                        val = (int64_t)rval;
                    } break;
                    case 6: /* lwu */
                    {
                        uint32_t rval;
                        if (target_read_u32(s, &rval, addr))
                            goto mmu_exception;
                        val = rval;
                    } break;
#endif
#if XLEN >= 128
                    case 7: /* ldu */
                    {
                        uint64_t rval;
                        if (target_read_u64(s, &rval, addr))
                            goto mmu_exception;
                        val = rval;
                    } break;
#endif
                    default: ILLEGAL_INSTR("025")
                }
                if (rd != 0)
                    write_reg(rd, val);
                NEXT_INSN;
            case 0x23: /* store */
                funct3 = (insn >> 12) & 7;
                imm    = rd | ((insn >> (25 - 5)) & 0xfe0);
                imm    = (imm << 20) >> 20;
                addr   = read_reg(rs1) + imm;
                val    = read_reg(rs2);

                switch (funct3) {
                    case 0: /* sb */
                        if (target_write_u8(s, addr, val))
                            goto mmu_exception;
                        break;
                    case 1: /* sh */
                        if (target_write_u16(s, addr, val))
                            goto mmu_exception;
                        break;
                    case 2: /* sw */
                        if (target_write_u32(s, addr, val))
                            goto mmu_exception;
                        break;
#if XLEN >= 64
                    case 3: /* sd */
                        if (target_write_u64(s, addr, val))
                            goto mmu_exception;
                        break;
#endif
#if XLEN >= 128
                    case 4: /* sq */
                        if (target_write_u128(s, addr, val))
                            goto mmu_exception;
                        break;
#endif
                    default: ILLEGAL_INSTR("026")
                }
                NEXT_INSN;
            case 0x13:
                funct3   = (insn >> 12) & 7;
                imm      = (int32_t)insn >> 20;
                val      = read_reg(rs1);
                _funct12 = (insn >> 20) & 0xFFF;
                _funct7  = (insn >> 25) & 0x7F;
                _funct6  = (insn >> 26) & 0x3F;
                _shamt   = (insn >> 20) & (XLEN == 32 ? 0x1F : 0x3F);

                switch (funct3) {
                    case 0: /* addi */ val = (intx_t)(read_reg(rs1) + imm); break;
                    case 1: /* cpop */
                        if (EN_ZBB && (_funct12 == 0x601)) {

                            CAPTURED_INSTR("CTZ");
                            val = (intx_t)glue(ctz_,XLEN)((intx_t)read_reg(rs1));

                        } else if (EN_ZBB && (_funct12 == 0x600)) {

                            CAPTURED_INSTR("CLZ");
                            val = (intx_t)glue(clz,XLEN)((intx_t)read_reg(rs1));

                        } else if (EN_ZBB && (_funct12 == 0x602)) {

                            CAPTURED_INSTR("CPOP");
                            val = (intx_t)glue(cpop,XLEN)((uintx_t)read_reg(rs1));

                        } else if ( EN_ZBB && (_funct12 == 0x605)) {

                            CAPTURED_INSTR("SEXT.H");
                            val = (intx_t)glue(sext_h,XLEN)((intx_t)read_reg(rs1));

                        } else if ( EN_ZBB && (_funct12 == 0x604)) {

                            CAPTURED_INSTR("SEXT.B");
                            val = (intx_t)glue(sext_b,XLEN)((intx_t)read_reg(rs1));

                        } else if (EN_ZBS && ((XLEN == 32 && _funct7 == 0x14)
                                          ||  (XLEN >= 64 && _funct6 == 0x0A))) {

                            CAPTURED_INSTR("BSETI");
                            val = (uintx_t)val | ((uintx_t)1 << (_shamt & (XLEN - 1)));

                        } else if (EN_ZBS && ((XLEN == 32 && _funct7 == 0x24)
                                          ||  (XLEN >= 64 && _funct6 == 0x12))) {

                            CAPTURED_INSTR("BCLRI");
                            val = (uintx_t)val & ~((uintx_t)1 << (_shamt & (XLEN - 1)));

                        } else if (EN_ZBS && ((XLEN == 32 && _funct7 == 0x34)
                                          ||  (XLEN >= 64 && _funct6 == 0x1A))) {

                            CAPTURED_INSTR("BINVI");
                            val = (uintx_t)val ^ ((uintx_t)1 << (_shamt & (XLEN - 1)));

                        } else if ((imm & ~(XLEN - 1)) != 0) {

                            ILLEGAL_INSTR("027")

                        } else {

                            val = (intx_t)(read_reg(rs1) << (imm & (XLEN - 1)));

                        }
                        break;
                    case 2: /* slti  */ val = (target_long)read_reg(rs1) < (target_long)imm; break;
                    case 3: /* sltiu */ val = read_reg(rs1) < (target_ulong)imm; break;
                    case 4: /* xori  */ val = read_reg(rs1) ^ imm; break;
                    case 5: 
                        if (EN_ZBS && ((XLEN == 32 && _funct7 == 0x24)
                                   ||  (XLEN >= 64 && _funct6 == 0x12))) {

                            CAPTURED_INSTR("BEXTI");
                            val = ((uintx_t)val >> (_shamt & (XLEN - 1))) & (uintx_t)1;

                          /* srli/srai */
                        } else if ((imm & ~((XLEN - 1) | 0x400)) != 0) {

                            ILLEGAL_INSTR("028")

                        } else if (imm & 0x400) {

                            val = (intx_t)read_reg(rs1) >> (imm & (XLEN - 1));

                        } else {

                            val = (intx_t)((uintx_t)read_reg(rs1) >> (imm & (XLEN - 1)));

                        }
                        break;
                    case 6: /* ori */ val = read_reg(rs1) | imm; break;
                    default:
                    case 7: /* andi */ val = read_reg(rs1) & imm; break;

                }
                if (rd != 0)
                    write_reg(rd, val);
                NEXT_INSN;
#if XLEN >= 64
            case 0x1b: /* OP-IMM-32 */
                funct3   = (insn >> 12) & 7;
                imm      = (int32_t)insn >> 20;
                val      = read_reg(rs1);
                _funct12 = (insn >> 20) & 0xFFF;
                _funct6  = (insn >> 26) & 0x3F;
                _shamt   = (insn >> 20) & 0x3F;

                switch (funct3) {
                    case 0: /* addiw */ val = (int32_t)(val + imm); break;
                    case 1:
                        if (EN_ZBA && (_funct6 == 0x2)) {

                            CAPTURED_INSTR("SLLI.UW");
                            val = (uintx_t)(uint32_t)val << _shamt;

                        } else if (EN_ZBB && (_funct12 == 0x601)) {

                            CAPTURED_INSTR("CTZW");
                            val = (intx_t)glue(ctzw,XLEN)((intx_t)read_reg(rs1));

                        } else if (EN_ZBB && (_funct12 == 0x602)) {

                            CAPTURED_INSTR("CPOPW");
                            val = (intx_t)glue(cpopw,XLEN)((intx_t)read_reg(rs1));

                        } else if (EN_ZBB && (_funct12 == 0x600)) {

                            CAPTURED_INSTR("CLZW");
                            val = (intx_t)glue(clzw,XLEN)((intx_t)read_reg(rs1));

                        } else if ((imm & ~31) != 0) {
                            ILLEGAL_INSTR("029")
                        } else {
                          /* slliw */
                          val = (int32_t)(val << (imm & 31));
                        }
                        break;
                    case 5: /* srliw/sraiw */
                        if ((imm & ~(31 | 0x400)) != 0)
                            ILLEGAL_INSTR("030")
                        if (imm & 0x400)
                            val = (int32_t)val >> (imm & 31);
                        else
                            val = (int32_t)((uint32_t)val >> (imm & 31));
                        break;
                    default: ILLEGAL_INSTR("031")
                }
                if (rd != 0)
                    write_reg(rd, val);
                NEXT_INSN;

            //FIXME: This comment was found here: case 0x5b: /* OP-IMM-32 */

            case 0x5b: /* Andes CUSTOM-2 */

                _funct7  = (insn >> 25) & 0x7F;  //31:27
                _bit30   = (insn >> 30) & 0x1;
                _funct3  = (insn >> 12) & 0x7;   //14:12

                zero32_rs2 = read_reg(rs2) & 0xFFFFFFFF;

                isBranchGroup = false;

                if(_funct3 == 0x0) { //LEA group
                    switch(_funct7) {
                      case 0x05: CAPTURED_INSTR("A* LEA.h"); break;
                                 val = read_reg(rs1) + (read_reg(rs2) << 1);
                                 break;
                      case 0x06: CAPTURED_INSTR("A* LEA.w");
                                 val = read_reg(rs1) + (read_reg(rs2) << 2);
                                 break;
                      case 0x07: CAPTURED_INSTR("A* LEA.d");
                                 val = read_reg(rs1) + (read_reg(rs2) << 3);
                                 break;
                      case 0x08: CAPTURED_INSTR("A* LEA.b.ze");
                                 val = read_reg(rs1) + zero32_rs2;
                                 break;
                      case 0x09: CAPTURED_INSTR("A* LEA.h.ze");
                                 val = read_reg(rs1) + (zero32_rs2 << 1);
                                 break;
                      case 0x0a: CAPTURED_INSTR("A* LEA.w.ze");
                                 val = read_reg(rs1) + (zero32_rs2 << 2);
                                 break;
                      case 0x0b: CAPTURED_INSTR("A* LEA.d.ze");
                                 val = read_reg(rs1) + (zero32_rs2 << 3);
                                 break;
                      default: ILLEGAL_INSTR("CUST2-1");
                    }

                    if (rd != 0) write_reg(rd, val);

                } else if(_funct3 == 0x2) { //BFOZ

                    CAPTURED_INSTR("A* BFOZ RV64");
                    val = bfoz_oper(insn,read_reg(rs1));
                    if (rd != 0) write_reg(rd, val);
                    //ILLEGAL_INSTR("A* BFOZ RV64"); need test

                } else if(_funct3 == 0x3) { //BFOS 

                    CAPTURED_INSTR("A* BFOS RV64");
                    val = bfos_oper(insn,read_reg(rs1));
                    if (rd != 0) write_reg(rd, val);
                    //ILLEGAL_INSTR("A* BFOS RV64"); need test
                       
                } else { //Branch group

                  isBranchGroup = true;

                  cimm7 = ( get_field1(insn,30,6,6)
                          | get_field1(insn, 7,5,5)
                          | get_field1(insn,20,0,4));

                  simm11 =  sext( get_field1(insn,31,10,10)
                                | get_field1(insn,25,5,9)
                                | get_field1(insn,8, 1,4),10);

                  cimm6 = ( get_field1(insn, 7,5,5)
                          | get_field1(insn,20,0,4));

                  _new_pc = (target_ulong)(GET_PC() + simm11);

                  switch(_funct3) {

                      case 0x0: ILLEGAL_INSTR("CUST2-2-LEA");  break; //LEA  handled above
                      case 0x1: ILLEGAL_INSTR("CUST2-2-???1"); break; //???
                      case 0x2: ILLEGAL_INSTR("CUST2-2-BFOZ"); break; //BFOZ RV64 handled above
                      case 0x3: ILLEGAL_INSTR("CUST2-2-BFOS"); break; //BFOS RV64"handled above);
                      case 0x4: ILLEGAL_INSTR("CUST2-2-???2"); break; //???

                      case 0x5: CAPTURED_INSTR("A* BEQC");

                                cond = (read_reg(rs1) == cimm7);
                                break;

                      case 0x6: CAPTURED_INSTR("A* BNEC");

                                cond = (read_reg(rs1) != cimm7); break;
                                break;

                      case 0x7: if(_bit30 == 0) {

                                  CAPTURED_INSTR("A* BBC RV64");
                                  cond = (((read_reg(rs1) >> cimm6) & 0x1) == 0x0); //bit cleared
                                  if(!cond) { NEXT_INSN; }

                                } else if(_bit30 == 1) {

                                  CAPTURED_INSTR("A* BBS RV64");
                                  cond = (((read_reg(rs1) >> cimm6) & 0x1) == 0x1); //bit set
                                  if(!cond) { NEXT_INSN; }
                                }
                                break;
                      default: ILLEGAL_INSTR("CUST2-2"); break;
                  }

                  if(cond) {
                    if (!(s->misa & MCPUID_C) && (_new_pc & 3) != 0) {
                        s->pending_exception = CAUSE_MISALIGNED_FETCH;
                        s->pending_tval      = 0;
                        goto exception;
                    }
                    //Elsewhere new_pc is calculated and then recalculated here. FIXME: Is there a reason?
                    //s->pc = (intx_t)(GET_PC() + simm11);
                    s->pc = _new_pc;
                    JUMP_INSN(ctf_taken_branch);
                  } 

                } //end of branch group

                if(!isBranchGroup) { NEXT_INSN; }
               
#endif


#if XLEN >= 128
            case 0x5b: /* OP-IMM-64 */
                funct3 = (insn >> 12) & 7;
                imm    = (int32_t)insn >> 20;
                val    = read_reg(rs1);
                switch (funct3) {
                    case 0: /* addid */ val = (int64_t)(val + imm); break;
                    case 1: /* sllid */
                        if ((imm & ~63) != 0)
                            ILLEGAL_INSTR("032")
                        val = (int64_t)(val << (imm & 63));
                        break;
                    case 5: /* srlid/sraid */
                        if ((imm & ~(63 | 0x400)) != 0)
                            ILLEGAL_INSTR("033")
                        if (imm & 0x400)
                            val = (int64_t)val >> (imm & 63);
                        else
                            val = (int64_t)((uint64_t)val >> (imm & 63));
                        break;
                    default: ILLEGAL_INSTR("034")
                }
                if (rd != 0)
                    write_reg(rd, val);
                NEXT_INSN;
#endif
            case 0x33:
                imm  = insn >> 25;
                val  = read_reg(rs1);
                val2 = read_reg(rs2);

                //These are defined separately to avoid clashing with other vars of similar name.
                //The imm variable name is unfortunate and confusing
                _funct12 = (insn >> 25) & 0xFFF; (void) _funct12;
                _funct7  = (insn >> 25) & 0x7F; (void) _funct7;
                _funct3  = (insn >> 12) & 0x7;  (void) _funct3;

                if (EN_ZBA && _funct7 == 0x10) {

                   switch (_funct3) {
                        case 0x2: CAPTURED_INSTR("SH1ADD");
                                  val = ((uintx_t)val << 1) + (uintx_t)val2;
                                  break;
                        case 0x4: CAPTURED_INSTR("SH2ADD");
                                  val = ((uintx_t)val << 2) + (uintx_t)val2;
                                  break;
                        case 0x6: CAPTURED_INSTR("SH3ADD");
                                  val = ((uintx_t)val << 3) + (uintx_t)val2;
                                  break;
                        default:  ILLEGAL_INSTR("ZBA001")
                    }

                } else if (EN_ZBB && _funct7 == 0x05 && _funct3 == 0x4) {

                    CAPTURED_INSTR("MIN");
                    val = ((intx_t)val < (intx_t)val2)   ? val : val2;

                //Can't easily use switch, EN_ZBC::CLMULx shares _funct7 = 0x5
                } else if (EN_ZBB && _funct7 == 0x05 && _funct3 == 0x5) {

                    CAPTURED_INSTR("MINU");
                    val = ((uintx_t)val < (uintx_t)val2) ? val : val2;

                } else if (EN_ZBB && _funct7 == 0x05 && _funct3 == 0x6) {

                    CAPTURED_INSTR("MAX");
                    val = ((intx_t)val < (intx_t)val2)   ? val2 : val;

                } else if (EN_ZBB && _funct7 == 0x05 && _funct3 == 0x7) {

                    CAPTURED_INSTR("MAXU");
                    val = ((uintx_t)val < (uintx_t)val2) ? val2 : val;

                } else if (EN_ZBB && _funct7 == 0x20 && _funct3 == 0x4) {

                    CAPTURED_INSTR("XNOR");
                    val = ~(val ^ val2);

                } else if (EN_ZBB && _funct7 == 0x20 && _funct3 == 0x6) {

                    CAPTURED_INSTR("ORN");
                    val = val | ~val2;

                } else if (EN_ZBB && _funct7 == 0x20 && _funct3 == 0x6) {

                    CAPTURED_INSTR("ANDN");
                    val = val & ~val2;

                } else if (EN_ZBC && _funct7 == 0x5 && _funct3 == 0x1) {

                    CAPTURED_INSTR("CLMUL");
                    val = (intx_t)glue(clmul, XLEN)(val, val2);

                } else if (EN_ZBC && _funct7 == 0x5 && _funct3 == 0x2) {

                    CAPTURED_INSTR("CLMULR");
                    val = (intx_t)glue(clmulr, XLEN)(val, val2);

                } else if (EN_ZBC && _funct7 == 0x5 && _funct3 == 0x3) {

                    CAPTURED_INSTR("CLMULH");
                    val = (intx_t)glue(clmulh, XLEN)(val, val2);

                } else if (EN_ZBS && _funct7 == 0x14 && _funct3 == 0x1) {

                    CAPTURED_INSTR("BSET");
                    val = (uintx_t)val | ((uintx_t)1 << (val2 & (XLEN - 1)));

                } else if (EN_ZBS && _funct7 == 0x24 && _funct3 == 0x1) {

                    CAPTURED_INSTR("BCLR");
                    val = (uintx_t)val & ~((uintx_t)1 << (val2 & (XLEN - 1)));

                } else if (EN_ZBS && _funct7 == 0x24 && _funct3 == 0x5) {

                    CAPTURED_INSTR("BEXT");
                    val = ((uintx_t)val >> (val2 & (XLEN - 1))) & (uintx_t)1;

                } else if (EN_ZBS && _funct7 == 0x34 && _funct3 == 0x1) {

                    CAPTURED_INSTR("BINV");
                    val = (uintx_t)val ^ ((uintx_t)1 << (val2 & (XLEN - 1)));

                } else if (imm == 1) {
                    //TODO CAN BE REMOVED - this is the original code before zbb
                    funct3 = (insn >> 12) & 7;
                    switch (funct3) {
                        case 0: /* mul */ val = (intx_t)((intx_t)val * (intx_t)val2); break;
                        case 1: /* mulh */ val = (intx_t)glue(mulh, XLEN)(val, val2); break;
                        case 2: /* mulhsu */ val = (intx_t)glue(mulhsu, XLEN)(val, val2); break;
                        case 3: /* mulhu */ val = (intx_t)glue(mulhu, XLEN)(val, val2); break;
                        case 4: /* div */ val = glue(div, XLEN)(val, val2); break;
                        case 5: /* divu */ val = (intx_t)glue(divu, XLEN)(val, val2); break;
                        case 6: /* rem */ val = glue(rem, XLEN)(val, val2); break;
                        case 7: /* remu */ val = (intx_t)glue(remu, XLEN)(val, val2); break;
                        default: ILLEGAL_INSTR("035")
                    }
                } else {
                    if (imm & ~0x20) {
                        ILLEGAL_INSTR("036")
                    }
                    funct3 = ((insn >> 12) & 7) | ((insn >> (30 - 3)) & (1 << 3));
                    switch (funct3) {
                        case 0: /* add */ val = (intx_t)(val + val2); break;
                        case 0 | 8: /* sub */ val = (intx_t)(val - val2); break;
                        case 1: /* sll */ val = (intx_t)(val << (val2 & (XLEN - 1))); break;
                        case 2: /* slt */ val = (target_long)val < (target_long)val2; break;
                        case 3: /* sltu */ val = val < val2; break;
                        case 4: /* xor */ val = val ^ val2; break;
                        case 5: /* srl */ val = (intx_t)((uintx_t)val >> (val2 & (XLEN - 1))); break;
                        case 5 | 8: /* sra */ val = (intx_t)val >> (val2 & (XLEN - 1)); break;
                        case 6: /* or */ val = val | val2; break;
                        case 7: /* and */ val = val & val2; break;
                        default: ILLEGAL_INSTR("037")
                    }
                }
                if (rd != 0)
                    write_reg(rd, val);
                NEXT_INSN;
#if XLEN >= 64
            case 0x3b: /* OP-32 */
                imm  = insn >> 25;
                val  = read_reg(rs1);
                val2 = read_reg(rs2);
                _funct12 = (insn >> 20) & 0xFFF;
                _funct7  = (insn >> 25) & 0x7F;
                _funct3  = (insn >> 12) & 0x7;

                if (EN_ZBA && _funct7 == 0x10 && _funct3 == 0x2) {
                    CAPTURED_INSTR("SH1ADD.UW");
                    val = ((uintx_t)(uint32_t)val << 1) + (uintx_t)val2;
                } else if (EN_ZBA && _funct7 == 0x10 && _funct3 == 0x4) {
                    CAPTURED_INSTR("SH2ADD.UW");
                    val = ((uintx_t)(uint32_t)val << 2) + (uintx_t)val2;
                } else if (EN_ZBA && _funct7 == 0x10 && _funct3 == 0x6) {
                    CAPTURED_INSTR("SH3ADD.UW");
                    val = ((uintx_t)(uint32_t)val << 3) + (uintx_t)val2;
                } else if (EN_ZBA && _funct7 == 0x04 && _funct3 == 0x0) {
                    CAPTURED_INSTR("ADD.UW/ZEXT.W");
                    val = (uintx_t)(uint32_t)val + (uintx_t)val2;
                } else if (EN_ZBB && _funct12 == 0x080 & _funct3 == 0x4) {
                   CAPTURED_INSTR("ZEXT.H");
                   val = val & HALF_WORD_MASK;
                } else if (imm == 1) {
                    funct3 = (insn >> 12) & 7;
                    switch (funct3) {
                        case 0: /* mulw */ val = (int32_t)((int32_t)val * (int32_t)val2); break;
                        case 4: /* divw */ val = div32(val, val2); break;
                        case 5: /* divuw */ val = (int32_t)divu32(val, val2); break;
                        case 6: /* remw */ val = rem32(val, val2); break;
                        case 7: /* remuw */ val = (int32_t)remu32(val, val2); break;
                        default: ILLEGAL_INSTR("038")
                    }
                } else {
                    if (imm & ~0x20) ILLEGAL_INSTR("039")
                    funct3 = ((insn >> 12) & 7) | ((insn >> (30 - 3)) & (1 << 3));
                    switch (funct3) {
                        case 0: /* addw */ val = (int32_t)(val + val2); break;
                        case 0 | 8: /* subw */ val = (int32_t)(val - val2); break;
                        case 1: /* sllw */ val = (int32_t)((uint32_t)val << (val2 & 31)); break;
                        case 5: /* srlw */ val = (int32_t)((uint32_t)val >> (val2 & 31)); break;
                        case 5 | 8: /* sraw */ val = (int32_t)val >> (val2 & 31); break;
                        default: ILLEGAL_INSTR("040")
                    }
                }
                if (rd != 0)
                    write_reg(rd, val);
                NEXT_INSN;
#endif

#if XLEN >= 128
            case 0x7b: /* OP-64 */
                imm  = insn >> 25;
                val  = read_reg(rs1);
                val2 = read_reg(rs2);
                if (imm == 1) {
                    funct3 = (insn >> 12) & 7;
                    switch (funct3) {
                        case 0: /* muld */ val = (int64_t)((int64_t)val * (int64_t)val2); break;
                        case 4: /* divd */ val = div64(val, val2); break;
                        case 5: /* divud */ val = (int64_t)divu64(val, val2); break;
                        case 6: /* remd */ val = rem64(val, val2); break;
                        case 7: /* remud */ val = (int64_t)remu64(val, val2); break;
                        default: ILLEGAL_INSTR("041")
                    }
                } else {
                    if (imm & ~0x20)
                        ILLEGAL_INSTR("042")
                    funct3 = ((insn >> 12) & 7) | ((insn >> (30 - 3)) & (1 << 3));
                    switch (funct3) {
                        case 0: /* addd */ val = (int64_t)(val + val2); break;
                        case 0 | 8: /* subd */ val = (int64_t)(val - val2); break;
                        case 1: /* slld */ val = (int64_t)((uint64_t)val << (val2 & 63)); break;
                        case 5: /* srld */ val = (int64_t)((uint64_t)val >> (val2 & 63)); break;
                        case 5 | 8: /* srad */ val = (int64_t)val >> (val2 & 63); break;
                        default: ILLEGAL_INSTR("043")
                    }
                }
                if (rd != 0)
                    write_reg(rd, val);
                NEXT_INSN;
#endif
            case 0x73:
                funct3 = (insn >> 12) & 7;
                imm    = insn >> 20;
                if (funct3 & 4)
                    val = rs1;
                else
                    val = read_reg(rs1);
                funct3 &= 3;
                switch (funct3) {
                    case 1: /* csrrw */
                        s->insn_counter = GET_INSN_COUNTER();
                        if (!s->stop_the_counter) {
                            int delta = s->insn_counter - insn_counter_start;
                            assert(delta >= 0);
                            s->mcycle += delta;
                            s->minstret += delta;
                        }
                        if (csr_read(s, funct3, &val2, imm, TRUE))
                            ILLEGAL_INSTR("044")
                        val2 = (intx_t)val2;
                        err  = csr_write(s, funct3, imm, val);
                        if (err == -2)
                            goto mmu_exception;
                        if (err < 0)
                            ILLEGAL_INSTR("045")
                        if (rd != 0)
                            write_reg(rd, val2);
                        insn_counter_addend = s->insn_counter + n_cycles;
                        if (err > 0) {
                            s->pc = GET_PC() + 4;
                            if (err == 2)
                                JUMP_INSN(ctf_nop);
                            else
                                goto done_interp;
                        }
                        break;
                    case 2: /* csrrs */
                    case 3: /* csrrc */
                        s->insn_counter = GET_INSN_COUNTER();
                        if (!s->stop_the_counter) {
                            int delta = s->insn_counter - insn_counter_start;
                            assert(delta >= 0);
                            s->mcycle += delta;
                            s->minstret += delta;
                        }
                        if (csr_read(s, funct3, &val2, imm, (rs1 != 0)))
                            ILLEGAL_INSTR("046")
                        val2 = (intx_t)val2;
                        if (rs1 != 0) {
                            if (funct3 == 2)
                                val = val2 | val;
                            else
                                val = val2 & ~val;
                            err = csr_write(s, funct3, imm, val);
                            if (err == -2)
                                goto mmu_exception;
                            if (err < 0)
                                ILLEGAL_INSTR("047")
                        } else {
                            err = 0;
                        }
                        if (rd != 0)
                            write_reg(rd, val2);
                        if (err > 0) {
                            s->pc = GET_PC() + 4;
                            if (err == 2)
                                JUMP_INSN(ctf_nop);
                            else
                                goto done_interp;
                        }
                        break;
                    case 0:
                        switch (imm) {
                            case 0x000: /* ecall */
                                if (insn & 0x000fff80)
                                    ILLEGAL_INSTR("048")
                                s->pending_exception = CAUSE_USER_ECALL + s->priv;
                                s->pending_tval      = 0;
                                /* Intercept SBI_SHUTDOWN, that is, ecall with a7 == SBI_SHUTDOWN */
                                if (!s->ignore_sbi_shutdown && s->priv == PRV_M && read_reg(0x17) == SBI_SHUTDOWN)
                                    s->terminate_simulation = 1;
                                goto exception;
                            case 0x001: /* ebreak */
                                if (insn & 0x000fff80)
                                    ILLEGAL_INSTR("049")
                                s->pending_exception = CAUSE_BREAKPOINT;
                                s->pending_tval      = 0;
                                goto exception;
                            case 0x102: /* sret */
                            {
                                if (insn & 0x000fff80)
                                    ILLEGAL_INSTR("050")
                                if (s->priv < PRV_S)
                                    ILLEGAL_INSTR("051")
                                if (s->priv == PRV_S && s->mstatus & MSTATUS_TSR)
                                    ILLEGAL_INSTR("052")
                                s->pc = GET_PC();
                                handle_sret(s);
                                goto done_interp;
                            } break;
                            case 0x302: /* mret */
                            {
                                if (insn & 0x000fff80)
                                    ILLEGAL_INSTR("053")
                                if (s->priv < PRV_M)
                                    ILLEGAL_INSTR("054")
                                s->pc = GET_PC();
                                handle_mret(s);
                                goto done_interp;
                            } break;
                            case 0x7b2: /* dret */
                                if (!s->debug_mode)
                                    ILLEGAL_INSTR("055")
                                {
                                    if (insn & 0x000fff80)
                                        ILLEGAL_INSTR("056")
                                    if (s->priv
                                        < PRV_M)  // FIXME: It should be illegal even in M, but this is the only that we have now
                                        ILLEGAL_INSTR("057")
                                    s->pc = GET_PC();
                                    handle_dret(s);
                                    goto done_interp;
                                }
                                break;
                            case 0x105: /* wfi */
                                if (insn & 0x00007f80)
                                    ILLEGAL_INSTR("058")
                                if (s->priv == PRV_U)
                                    ILLEGAL_INSTR("059")
                                /* "When TW=1, if WFI is executed in S- mode, and
                                   it does not complete within an
                                   implementation-specific, bounded time limit,
                                   the WFI instruction causes an illegal
                                   instruction trap." */
                                if (s->priv == PRV_S && s->mstatus & MSTATUS_TW)
                                    ILLEGAL_INSTR("060")
                                /* go to power down if no enabled interrupts are
                                   pending */
                                if (((s->mip & s->mie) == 0) && (s->machine->common.pending_interrupt == -1)
                                    || !s->machine->common.cosim) {
                                    s->power_down_flag = TRUE;
                                    s->pc              = GET_PC() + 4;
                                    goto done_interp;
                                }
                                break;
                            default:
                                if ((imm >> 5) == 0x09) {
                                    /* sfence.vma */
                                    if (insn & 0x00007f80)
                                        ILLEGAL_INSTR("061")
                                    if (s->priv == PRV_U)
                                        ILLEGAL_INSTR("062")
                                    if (s->priv == PRV_S && s->mstatus & MSTATUS_TVM)
                                        ILLEGAL_INSTR("063")
                                    if (rs1 == 0) {
                                        tlb_flush_all(s);
                                    } else {
                                        tlb_flush_vaddr(s, read_reg(rs1));
                                    }
                                    /* the current code TLB may have been flushed */
                                    s->pc = GET_PC() + 4;
                                    JUMP_INSN(ctf_nop);
                                } else {
                                    ILLEGAL_INSTR("064")
                                }
                                break;
                        }
                        break;
                    default: ILLEGAL_INSTR("065")
                }
                NEXT_INSN;
            case 0x0f: /* misc-mem */
                funct3 = (insn >> 12) & 7;
                switch (funct3) {
                    case 0: /* fence */
                        /* all variantions are reserved for future use */
                        break;
                    case 1: /* fence.i */
                        /* all variantions are reserved for future use */
                        break;
#if XLEN >= 128
                    case 2: /* lq */
                        imm  = (int32_t)insn >> 20;
                        addr = read_reg(rs1) + imm;
                        if (target_read_u128(s, &val, addr))
                            goto mmu_exception;
                        if (rd != 0)
                            write_reg(rd, val);
                        break;
#endif
                    default: ILLEGAL_INSTR("066")
                }
                NEXT_INSN;
            case 0x2f: funct3 = (insn >> 12) & 7;
#define OP_A(size)                                                                      \
    {                                                                                   \
        uint##size##_t rval;                                                            \
                                                                                        \
        addr   = read_reg(rs1);                                                         \
        funct3 = insn >> 27;                                                            \
        switch (funct3) {                                                               \
            case 2: /* lr.w/lr.d */                                                     \
                if (rs2 != 0)                                                           \
                    goto illegal_insn;                                                  \
                if (target_read_u##size(s, &rval, addr))                                \
                    goto mmu_exception;                                                 \
                val         = (int##size##_t)rval;                                      \
                s->load_res = addr;                                                     \
                s->load_res_memseqno = s->machine->memseqno;                            \
                break;                                                                  \
                                                                                        \
            case 3: /* sc.w/sc.d */                                                     \
                if ((addr & (size / 8 - 1)) != 0) {                                     \
                    s->pending_tval      = addr;                                        \
                    s->pending_exception = CAUSE_MISALIGNED_STORE;                      \
                    goto mmu_exception;                                                 \
                }                                                                       \
                if (s->load_res == addr && s->load_res_memseqno == s->machine->memseqno) { \
                    if (target_write_u##size(s, addr, read_reg(rs2)))                   \
                        goto mmu_exception;                                             \
                    val         = 0;                                                    \
                    s->load_res = ~0;                                                   \
                    s->load_res_memseqno = 0;                                           \
                } else {                                                                \
                    val = 1;                                                            \
                }                                                                       \
                break;                                                                  \
            case 1:    /* amiswap.w */                                                  \
            case 0:    /* amoadd.w */                                                   \
            case 4:    /* amoxor.w */                                                   \
            case 0xc:  /* amoand.w */                                                   \
            case 0x8:  /* amoor.w */                                                    \
            case 0x10: /* amomin.w */                                                   \
            case 0x14: /* amomax.w */                                                   \
            case 0x18: /* amominu.w */                                                  \
            case 0x1c: /* amomaxu.w */                                                  \
                if (target_read_u##size(s, &rval, addr)) {                              \
                    if (s->pending_exception != CAUSE_BREAKPOINT)                       \
                        s->pending_exception += 2; /* LD -> ST */                       \
                    goto mmu_exception;                                                 \
                }                                                                       \
                val  = (int##size##_t)rval;                                             \
                val2 = read_reg(rs2);                                                   \
                switch (funct3) {                                                       \
                    case 1: /* amiswap.w */ break;                                      \
                    case 0: /* amoadd.w */ val2 = (int##size##_t)(val + val2); break;   \
                    case 4: /* amoxor.w */ val2 = (int##size##_t)(val ^ val2); break;   \
                    case 0xc: /* amoand.w */ val2 = (int##size##_t)(val & val2); break; \
                    case 0x8: /* amoor.w */ val2 = (int##size##_t)(val | val2); break;  \
                    case 0x10: /* amomin.w */                                           \
                        if ((int##size##_t)val < (int##size##_t)val2)                   \
                            val2 = (int##size##_t)val;                                  \
                        break;                                                          \
                    case 0x14: /* amomax.w */                                           \
                        if ((int##size##_t)val > (int##size##_t)val2)                   \
                            val2 = (int##size##_t)val;                                  \
                        break;                                                          \
                    case 0x18: /* amominu.w */                                          \
                        if ((uint##size##_t)val < (uint##size##_t)val2)                 \
                            val2 = (int##size##_t)val;                                  \
                        break;                                                          \
                    case 0x1c: /* amomaxu.w */                                          \
                        if ((uint##size##_t)val > (uint##size##_t)val2)                 \
                            val2 = (int##size##_t)val;                                  \
                        break;                                                          \
                    default: goto illegal_insn;                                         \
                }                                                                       \
                if (target_write_u##size(s, addr, val2))                                \
                    goto mmu_exception;                                                 \
                break;                                                                  \
            default: goto illegal_insn;                                                 \
        }                                                                               \
    }

                switch (funct3) {
                    case 2: OP_A(32); break;
#if XLEN >= 64
                    case 3: OP_A(64); break;
#endif
#if XLEN >= 128
                    case 4: OP_A(128); break;
#endif
                    default: ILLEGAL_INSTR("067")
                }
                if (rd != 0)
                    write_reg(rd, val);
                NEXT_INSN;
            case 0x07:
                funct3 = (insn >> 12) & 7;
                imm    = (int32_t)insn >> 20;
                addr   = read_reg(rs1) + imm;
                switch (funct3) {
#if FLEN > 0
                    case 2: /* flw */
                    {
                        if (s->fs == 0)
                            ILLEGAL_INSTR("068")
                        uint32_t rval;
                        if (target_read_u32(s, &rval, addr))
                            goto mmu_exception;
                        write_fp_reg(rd, rval | F32_HIGH);
                    } break;
#if FLEN >= 64
                    case 3: /* fld */
                    {
                        if (s->fs == 0)
                            ILLEGAL_INSTR("069")
                        uint64_t rval;
                        if (target_read_u64(s, &rval, addr))
                            goto mmu_exception;
                        write_fp_reg(rd, rval | F64_HIGH);
                    } break;
#endif
#if FLEN >= 128
                    case 4: /* flq */
                    {
                        if (s->fs == 0)
                            ILLEGAL_INSTR("070")
                        uint128_t rval;
                        if (target_read_u128(s, &rval, addr))
                            goto mmu_exception;
                        write_fp_reg(rd, rval);
                    } break;
#endif
#endif  // FLEN > 0
#if VLEN > 0
                    /* Vector loads */
                    case 0:
                    case 5:
                    case 6:
                    case 7:
                        if (s->vs == 0)
                            ILLEGAL_INSTR("071")
                        vmem_result = vmem_op(s, insn, true, v_load_config);
                        if (vmem_result == 2)
                            goto mmu_exception;
                        else if (vmem_result == 1) {
                            s->vtype = VILL;
                            ILLEGAL_INSTR("072")
                        }
                        break;
#endif
                    default: ILLEGAL_INSTR("073")
                }
                NEXT_INSN;
            case 0x27:
                funct3 = (insn >> 12) & 7;
                imm    = rd | ((insn >> (25 - 5)) & 0xfe0);
                imm    = (imm << 20) >> 20;
                addr   = read_reg(rs1) + imm;
                switch (funct3) {
#if FLEN > 0
                    case 2: /* fsw */
                        if (s->fs == 0)
                            ILLEGAL_INSTR("074")
                        if (target_write_u32(s, addr, read_fp_reg(rs2)))
                            goto mmu_exception;
                        break;
#if FLEN >= 64
                    case 3: /* fsd */
                        if (s->fs == 0)
                            ILLEGAL_INSTR("075")
                        if (target_write_u64(s, addr, read_fp_reg(rs2)))
                            goto mmu_exception;
                        break;
#endif
#if FLEN >= 128
                    case 4: /* fsq */
                        if (s->fs == 0)
                            ILLEGAL_INSTR("076")
                        if (target_write_u128(s, addr, read_fp_reg(rs2)))
                            goto mmu_exception;
                        break;
#endif

#endif  // FLEN > 0
#if VLEN > 0
                    /* vector stores */
                    case 0:
                    case 5:
                    case 6:
                    case 7:
                        if (s->vs == 0)
                            ILLEGAL_INSTR("077")
                        vmem_result = vmem_op(s, insn, false, v_store_config);
                        if (vmem_result == 2)
                            goto mmu_exception;
                        else if (vmem_result == 1) {
                            s->vtype = VILL;
                            ILLEGAL_INSTR("078")
                        }
                        break;
#endif
                    default: ILLEGAL_INSTR("079")
                }
                NEXT_INSN;
#if FLEN > 0
            case 0x43: /* fmadd */
                if (s->fs == 0)
                    ILLEGAL_INSTR("080")
                funct3 = (insn >> 25) & 3;
                rs3    = insn >> 27;
                rm     = get_insn_rm(s, (insn >> 12) & 7);
                if (rm < 0)
                    ILLEGAL_INSTR("081")
                switch (funct3) {
                    case 0:
                        write_fp_reg(rd,
                                     (fp_uint)fma_sf32(chkfp32(read_fp_reg(rs1)),
                                                       chkfp32(read_fp_reg(rs2)),
                                                       chkfp32(read_fp_reg(rs3)),
                                                       (RoundingModeEnum)rm,
                                                       &s->fflags)
                                         | F32_HIGH);
                        break;
#if FLEN >= 64
                    case 1:
                        write_fp_reg(
                            rd,
                            (fp_uint)
                                    fma_sf64(read_fp_reg(rs1), read_fp_reg(rs2), read_fp_reg(rs3), (RoundingModeEnum)rm, &s->fflags)
                                | F64_HIGH);
                        break;
#endif
#if FLEN >= 128
                    case 3:
                        write_fp_reg(
                            rd,
                            (fp_uint)
                                fma_sf128(read_fp_reg(rs1), read_fp_reg(rs2), read_fp_reg(rs3), (RoundingModeEnum)rm, &s->fflags));
                        ;
#endif
                    default: ILLEGAL_INSTR("082")
                }
                NEXT_INSN;
            case 0x47: /* fmsub */
                if (s->fs == 0)
                    ILLEGAL_INSTR("083")
                funct3 = (insn >> 25) & 3;
                rs3    = insn >> 27;
                rm     = get_insn_rm(s, (insn >> 12) & 7);
                if (rm < 0)
                    ILLEGAL_INSTR("084")
                switch (funct3) {
                    case 0:
                        write_fp_reg(rd,
                                     fma_sf32(chkfp32(read_fp_reg(rs1)),
                                              chkfp32(read_fp_reg(rs2)),
                                              chkfp32(read_fp_reg(rs3)) ^ FSIGN_MASK32,
                                              (RoundingModeEnum)rm,
                                              &s->fflags)
                                         | F32_HIGH);
                        break;
#if FLEN >= 64
                    case 1:
                        write_fp_reg(rd,
                                     fma_sf64(read_fp_reg(rs1),
                                              read_fp_reg(rs2),
                                              read_fp_reg(rs3) ^ FSIGN_MASK64,
                                              (RoundingModeEnum)rm,
                                              &s->fflags)
                                         | F64_HIGH);
                        break;
#endif
#if FLEN >= 128
                    case 3:
                        write_fp_reg(rd,
                                     fma_sf128(read_fp_reg(rs1),
                                               read_fp_reg(rs2),
                                               read_fp_reg(rs3) ^ FSIGN_MASK128,
                                               (RoundingModeEnum)rm,
                                               &s->fflags));
                        break;
#endif
                    default: ILLEGAL_INSTR("085")
                }
                NEXT_INSN;
            case 0x4b: /* fnmsub */
                if (s->fs == 0)
                    ILLEGAL_INSTR("086")
                funct3 = (insn >> 25) & 3;
                rs3    = insn >> 27;
                rm     = get_insn_rm(s, (insn >> 12) & 7);
                if (rm < 0)
                    ILLEGAL_INSTR("087")
                switch (funct3) {
                    case 0:
                        write_fp_reg(rd,
                                     fma_sf32(chkfp32(read_fp_reg(rs1)) ^ FSIGN_MASK32,
                                              chkfp32(read_fp_reg(rs2)),
                                              chkfp32(read_fp_reg(rs3)),
                                              (RoundingModeEnum)rm,
                                              &s->fflags)
                                         | F32_HIGH);
                        break;
#if FLEN >= 64
                    case 1:
                        write_fp_reg(rd,
                                     fma_sf64(read_fp_reg(rs1) ^ FSIGN_MASK64,
                                              read_fp_reg(rs2),
                                              read_fp_reg(rs3),
                                              (RoundingModeEnum)rm,
                                              &s->fflags)
                                         | F64_HIGH);
                        break;
#endif
#if FLEN >= 128
                    case 3:
                        write_fp_reg(rd,
                                     fma_sf128(read_fp_reg(rs1) ^ FSIGN_MASK128,
                                               read_fp_reg(rs2),
                                               read_fp_reg(rs3),
                                               (RoundingModeEnum)rm,
                                               &s->fflags));
                        break;
#endif
                    default: ILLEGAL_INSTR("088")
                }
                NEXT_INSN;
            case 0x4f: /* fnmadd */
                if (s->fs == 0)
                    ILLEGAL_INSTR("089")
                funct3 = (insn >> 25) & 3;
                rs3    = insn >> 27;
                rm     = get_insn_rm(s, (insn >> 12) & 7);
                if (rm < 0)
                    ILLEGAL_INSTR("090")
                switch (funct3) {
                    case 0:
                        write_fp_reg(rd,
                                     fma_sf32(chkfp32(read_fp_reg(rs1)) ^ FSIGN_MASK32,
                                              chkfp32(read_fp_reg(rs2)),
                                              chkfp32(read_fp_reg(rs3)) ^ FSIGN_MASK32,
                                              (RoundingModeEnum)rm,
                                              &s->fflags)
                                         | F32_HIGH);
                        break;
#if FLEN >= 64
                    case 1:
                        write_fp_reg(rd,
                                     fma_sf64(read_fp_reg(rs1) ^ FSIGN_MASK64,
                                              read_fp_reg(rs2),
                                              read_fp_reg(rs3) ^ FSIGN_MASK64,
                                              (RoundingModeEnum)rm,
                                              &s->fflags)
                                         | F64_HIGH);
                        break;
#endif
#if FLEN >= 128
                    case 3:
                        write_fp_reg(rd,
                                     fma_sf128(read_fp_reg(rs1) ^ FSIGN_MASK128,
                                               read_fp_reg(rs2),
                                               read_fp_reg(rs3) ^ FSIGN_MASK128,
                                               (RoundingModeEnum)rm,
                                               &s->fflags));
                        break;
#endif
                    default: ILLEGAL_INSTR("091")
                }
                NEXT_INSN;
            case 0x53:
                if (s->fs == 0)
                    ILLEGAL_INSTR("092")
                imm = insn >> 25;
                rm  = (insn >> 12) & 7;
                switch (imm) {
#define F_SIZE 32
#include "dromajo_fp_template.h"
#if FLEN >= 64
#define F_SIZE 64
#include "dromajo_fp_template.h"
#endif
#if FLEN >= 128
#define F_SIZE 128
#include "dromajo_fp_template.h"
#endif

                    default: ILLEGAL_INSTR("093")
                }
                NEXT_INSN;
#endif
#if VLEN > 0
            case 0x57:
                if (s->vs == 0)
                    ILLEGAL_INSTR("094")
                s->vs  = 3;
                vl     = s->vl;
                vm     = (insn >> 25) & 1;
                funct3 = (insn >> 12) & 7;
                funct6 = (insn >> 26) & 0x3F;
                switch (funct3) {
                    case 0: /* OPIVV */
                        switch (funct6) {
                            case 0: /* vadd.vv */
                                if (vectorize_arithmetic(s, rs2, rd, rs1, SINGLE_WIDTH, true, vm, v_add_config))
                                    ILLEGAL_INSTR("095")
                                break;
                            default: ILLEGAL_INSTR("096")
                        }
                        break;
                    case 2: /* OPMVV */
                        switch (funct6) {
                            case 0x30: /* vwaddu.vv */
                                if (vectorize_arithmetic(s, rs2, rd, rs1, WIDEN_VD, true, vm, vw_addu_config))
                                    ILLEGAL_INSTR("097")
                                break;
                            case 0x31: /* vwadd.vv */
                                if (vectorize_arithmetic(s, rs2, rd, rs1, WIDEN_VD, true, vm, vw_add_config))
                                    ILLEGAL_INSTR("098")
                                break;
                            case 0x34: /* vwaddu.wv */
                                if (vectorize_arithmetic(s, rs2, rd, rs1, WIDEN_VD_VS2, true, vm, vw_adduw_config))
                                    ILLEGAL_INSTR("099")
                                break;
                            case 0x35: /* vwadd.wv */
                                if (vectorize_arithmetic(s, rs2, rd, rs1, WIDEN_VD_VS2, true, vm, vw_addw_config))
                                    ILLEGAL_INSTR("100")
                                break;
                            default: ILLEGAL_INSTR("101")
                        }
                        break;
                    case 3: /* OPIVI */
                        switch (funct6) {
                            case 0: /* vadd.vi */
                                imm = rs1;
                                imm = imm << 27 >> 27;  // sign extend 5-bit immediate
                                if (vectorize_arithmetic(s, rs2, rd, imm, SINGLE_WIDTH, false, vm, v_add_config))
                                    ILLEGAL_INSTR("102")
                                break;
                            default: ILLEGAL_INSTR("103")
                        }
                        break;
                    case 4: /* OPIVX */
                        switch (funct6) {
                            case 0: /* vadd.vx */
                                if (vectorize_arithmetic(s, rs2, rd, read_reg(rs1), SINGLE_WIDTH, false, vm, v_add_config))
                                    ILLEGAL_INSTR("104")
                                break;
                            default: ILLEGAL_INSTR("105")
                        }
                        break;
                    case 6: /* OPMVX */
                        switch (funct6) {
                            case 0x30: /* vwaddu.vx */
                                if (vectorize_arithmetic(s, rs2, rd, read_reg(rs1), WIDEN_VD, false, vm, vw_addu_config))
                                    ILLEGAL_INSTR("106")
                                break;
                            case 0x31: /* vwadd.vx */
                                if (vectorize_arithmetic(s, rs2, rd, read_reg(rs1), WIDEN_VD, false, vm, vw_add_config))
                                    ILLEGAL_INSTR("107")
                                break;
                            case 0x34: /* vwaddu.wx */
                                if (vectorize_arithmetic(s, rs2, rd, read_reg(rs1), WIDEN_VD_VS2, false, vm, vw_adduw_config))
                                    ILLEGAL_INSTR("108")
                                break;
                            case 0x35: /* vwadd.wx */
                                if (vectorize_arithmetic(s, rs2, rd, read_reg(rs1), WIDEN_VD_VS2, false, vm, vw_addw_config))
                                    ILLEGAL_INSTR("109")
                                break;
                            default: ILLEGAL_INSTR("110")
                        }
                        break;
                    case 7: /* OPCFG */
                        vset = (insn >> 30) & 3;
                        avl  = 0;
                        switch (vset) {
                            case 0:
                            case 1: /* vsetvli */ rs2 = 0; s->vtype = (insn >> 20) & 0x7ff;
                            case 2:         /* vsetvl  */
                                if (rs2) {  // vill set, vl cleared if reserved bits are written by rs2
                                    target_ulong vtype = read_reg(rs2);
                                    if (vtype > 0xff || vtype & 0x20 || (vtype & 0x4) == 0x4) {
                                        s->vtype = VILL;
                                        break;
                                    }
                                    s->vtype = vtype;
                                }
                                if (rs1)  // Normal stripmining
                                    avl = read_reg(rs1);
                                else if (rd)  // Set vl to VLMAX
                                    avl = ~0;
                                else  // Keep existing vl
                                    /* XXX - pg 26: "Use of the instruction with a new SEW/LMUL
                                     * ration that would result in a change of VLMAX is reserved.
                                     * Implementations may set vill in this case."
                                     * Spike doesn't do this so neither do we, worth asking about */
                                    avl = vl;
                                break;
                            case 3: /* vsetivli*/
                                s->vtype = (insn >> 20) & 0x3ff;
                                avl      = (insn >> 15) & 0x1f;
                                break;
                        }

                        if (get_sew(s) >= VLEN * get_lmul(s) / 8) {  // vector must be more than one elm long
                            s->vtype = VILL;
                            s->vl    = 0;
                        } else if (avl <= get_vlmax(s))
                            s->vl = avl;
                        else
                            s->vl = get_vlmax(s);
                        if (rd)
                            write_reg(rd, s->vl);
                        break;
                }
                NEXT_INSN;
#endif
            default: ILLEGAL_INSTR("111")
        }
        /* update PC for next instruction */
    jump_insn:;

        if(s->machine->common.stf_trace) {
            if(stf_trace_trigger(s,GET_PC(),insn)) {
              s->pc = GET_PC();
              if(s->machine->common.stf_has_exit_pending) {
                  goto the_end;
              }
              return insn_executed;
            }
        }

    } /* end of main loop */
illegal_insn:

#if REPORT_ILLEGAL == 1
fprintf(dromajo_stderr,"ILLEGAL INSN FALLTHROUGH\n");
exit(1);
#endif

    s->pending_exception = CAUSE_ILLEGAL_INSTRUCTION;
    s->pending_tval      = 0;
mmu_exception:
exception:
    s->pc = GET_PC();
    if (s->pending_exception >= 0) {
        if (s->pending_exception < CAUSE_USER_ECALL || s->pending_exception > CAUSE_USER_ECALL + 3) {
            /* All other causes cancelled the instruction and shouldn't be
             * counted in minstret */
            --insn_counter_addend;
            --insn_executed;
        }

        raise_exception2(s, s->pending_exception, s->pending_tval);
    }
    /* we exit because XLEN may have changed */

done_interp:
    n_cycles--;

the_end:
    s->insn_counter = GET_INSN_COUNTER();
    if (!s->stop_the_counter) {
        int delta = s->insn_counter - insn_counter_start;
        assert(delta >= 0);
        s->mcycle += delta;
        s->minstret += delta;
    }

    return insn_executed;
}

#undef uintx_t
#undef intx_t
#undef XLEN
#undef OP_A
