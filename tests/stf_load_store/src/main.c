#include "util.h"
#include <stdio.h>
#include <stdint.h>

int main(int argc, char* argv[])
{
    START_TRACE;

    asm volatile (
        "li sp, 0x80000000;"       // Set stack pointer to an address
        "li t0, 0x123456789ABCDEF0;" // Load immediate 64-bit value into t0

        "sd t0, 0(sp);"            // Store t0 value to stack 

        // Load operations
        //"c.lb t1, 0(sp);"
        //"c.lbu t2, 0(sp);"
        //"c.ld t3, 0(sp);"
        //"c.ldsp t4, 0(sp);"
        //"c.lh t5, 0(sp);"
        //"c.lhu t6, 0(sp);"
        //"c.lw t0, 0(sp);"
        //"c.lwsp t1, 0(sp);"
        "fld ft0, 0(sp);"
        //"flh ft1, 0(sp);" // breaks dromajo Benchmark exited with code: 2675 
        "flw ft2, 0(sp);"
        "lb t0, 0(sp);"
        "lbu t0, 0(sp);"
        "ld t0, 0(sp);"
        "lh t0, 0(sp);"
        "lhu t0, 0(sp);"
        "lr.d t0, (sp);"
        "lr.w t0, (sp);"
        "lw t0, 0(sp);"
        "lwu t0, 0(sp);"

        // Store operations
        //"c.sb t0, 0(sp);"
        //"c.sd t0, 0(sp);"
        //"c.sh t0, 0(sp);"
        //"c.sw t0, 0(sp);"
        "fsd ft0, 0(sp);"
        //"fsh ft1, 0(sp);" // breaks dromajo Benchmark exited with code: 2675 
        "fsw ft2, 0(sp);"
        "sb t0, 0(sp);"
        "sc.d t1, t0, (sp);"
        "sc.w t1, t0, (sp);"
        "sd t0, 0(sp);"
        "sh t0, 0(sp);"
        "sw t0, 4(sp);"
        :
        :
        : "t0", "t1", "t2", "t3", "t4", "t5", "t6", "ft0", "ft1", "ft2"
    );

    STOP_TRACE;
    return 0;
}