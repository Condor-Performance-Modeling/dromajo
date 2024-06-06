#include "util.h"
#include <stdio.h>
#include <stdint.h>

int main(int argc, char* argv[])
{
    START_TRACE;

    asm volatile (
        // Load operations
        "li t0, 0x123456789ABCDEF0;" // Load immediate 64-bit value into t0
        "sd t0, 0(sp);"              // Store t0 value to stack for further load instructions

        "lb t0, 0(sp);"              // Load byte from stack into t0 (signed)
        "lbu t0, 0(sp);"             // Load byte from stack into t0 (unsigned)
        "lh t0, 0(sp);"              // Load halfword from stack into t0 (signed)
        "lhu t0, 0(sp);"             // Load halfword from stack into t0 (unsigned)
        "lw t0, 0(sp);"              // Load word from stack into t0 (signed)
        "lwu t0, 0(sp);"             // Load word from stack into t0 (unsigned)
        "ld t0, 0(sp);"              // Load doubleword from stack into t0

        // Store operations
        "sb t0, 0(sp);"              // Store byte from t0 to stack
        "sh t0, 0(sp);"              // Store halfword from t0 to stack
        "sw t0, 0(sp);"              // Store word from t0 to stack
        "sd t0, 0(sp);"              // Store doubleword from t0 to stack
        :
        :
        : "t0"
    );

    STOP_TRACE;
    return 0;
}
