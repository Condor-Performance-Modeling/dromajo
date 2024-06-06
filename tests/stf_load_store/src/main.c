#include "util.h"
#include <stdio.h>
#include <stdint.h>

//c.lb rd, offset(rs1)
//c.lbu rd, offset(rs1)
//c.ld rd, offset(rs1)
//c.ldsp rd, offset(sp)
//c.lh rd, offset(rs1)
//c.lhu rd, offset(rs1)
//c.lw rd, offset(rs1)
//c.lwsp rd, offset(sp)
//fld rd, offset(rs1)
//flh rd, offset(rs1)
//flw rd, offset(rs1)
//lb rd, offset(rs1)
//lbu rd, offset(rs1)
//ld rd, offset(rs1)
//lh rd, offset(rs1)
//lhu rd, offset(rs1)
//lr.d rd, (rs1)
//lr.w rd, (rs1)
//lw rd, offset(rs1)
//lwu rd, offset(rs1)
//
//c.sb rs2, offset(rs1)
//c.sd rs2, offset(rs1)
//c.sh rs2, offset(rs1)
//c.sw rs2, offset(rs1)
//fsd rs2, offset(rs1)
//fsh rs2, offset(rs1)
//fsw rs2, offset(rs1)
//sb rs2, offset(rs1)
//sc.d rd, rs2, (rs1)
//sc.w rd, rs2, (rs1)
//sd rs2, offset(rs1)
//sh rs2, offset(rs1)
//sw rs2, offset(rs1)

//FIXME: code should fit in 80 columns max
int main(int argc, char* argv[])
{
    START_TRACE;

    asm volatile (
        // Load operations
        //FIXME do not use pseudo ops you want 1:1 in the stf
        //"li t0, 0x123456789ABCDEF0;" // Load immediate 64-bit value into t0
        "lui t0, 0x12345"         // Load upper immediate
        "addi t0, t0, 0x678"      // Add the remaining bits of the high part
        "slli t0, t0, 32"         // SHL by 32 to prepare for the lower 32 bits
        "addi t0, t0, 0x9ABCDEF0" // Add the lower 32 bits

        //FIXME: SP is not initialized
        "sd t0, 0(sp);"           // Store t0 value to stack 

        //FIXME: there should be one of each of these
        //x c.lb rd, offset(rs1)
        //x c.lbu rd, offset(rs1)
        //x c.ld rd, offset(rs1)
        //x c.ldsp rd, offset(sp)
        //x c.lh rd, offset(rs1)
        //x c.lhu rd, offset(rs1)
        //x c.lw rd, offset(rs1)
        //x c.lwsp rd, offset(sp)
        //x fld rd, offset(rs1)
        //x flh rd, offset(rs1)
        //x flw rd, offset(rs1)
        //  lb rd, offset(rs1)
        //  lbu rd, offset(rs1)
        //  ld rd, offset(rs1)
        //  lh rd, offset(rs1)
        //  lhu rd, offset(rs1)
        //x lr.d rd, (rs1)
        //x lr.w rd, (rs1)
        //  lw rd, offset(rs1)
        //  lwu rd, offset(rs1)
        "lb t0, 0(sp);"           // Load byte from stack into t0 (signed)
        "lbu t0, 0(sp);"          // Load byte from stack into t0 (unsigned)
        "lh t0, 0(sp);"           // Load halfword from stack into t0 (signed)
        "lhu t0, 0(sp);"          // Load halfword from stack into t0 (unsigned)
        "lw t0, 0(sp);"           // Load word from stack into t0 (signed)
        "lwu t0, 0(sp);"          // Load word from stack into t0 (unsigned)
        "ld t0, 0(sp);"           // Load doubleword from stack into t0

        // Store operations
        // FIXME: should be one of each of these
        //x c.sb rs2, offset(rs1)
        //x c.sd rs2, offset(rs1)
        //x c.sh rs2, offset(rs1)
        //x c.sw rs2, offset(rs1)
        //x fsd rs2, offset(rs1)
        //x fsh rs2, offset(rs1)
        //x fsw rs2, offset(rs1)
        //  sb rs2, offset(rs1)
        //x sc.d rd, rs2, (rs1)
        //x sc.w rd, rs2, (rs1)
        //  sd rs2, offset(rs1)
        //  sh rs2, offset(rs1)
        //  sw rs2, offset(rs1)
        "sb t0, 0(sp);"           // Store byte from t0 to stack
        "sh t0, 0(sp);"           // Store halfword from t0 to stack
        "sw t0, 0(sp);"           // Store word from t0 to stack
        "sd t0, 0(sp);"           // Store doubleword from t0 to stack
        :
        :
        : "t0"
    );

    STOP_TRACE;
    return 0;
}
