#include <stdio.h>
#include <stdint.h>

#ifdef __riscv

#ifndef START_TRACE
#define START_TRACE   asm volatile ("xor x0, x0, x0");
#endif

#ifndef STOP_TRACE
#define STOP_TRACE   asm volatile ("xor x0, x1, x1");
#endif

#else //#ifdef __riscv

// defined here to disable them for x86 linux test builds
#define START_TRACE
#define STOP_TRACE

#endif

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

int main(int argc, char* argv[])
{
    //Initialize memory - this instructions are not being traced
    asm volatile (
        "lui t0, 0x12345;"
        "addi t0, t0, 0x678;"
        "slli t0, t0, 8;" 
        "addi t0, t0, 0x9A;" 
        "slli t0, t0, 8;" 
        "addi t0, t0, 0xBC;" 
        "slli t0, t0, 8;" 
        "addi t0, t0, 0xDE;" 
        "slli t0, t0, 8;" 
        "addi t0, t0, 0xF0;" 

        "sd t0, 0(sp);"
    );
    
    START_TRACE;

    asm volatile (
        
        // Load operations
        //FIXME: there should be one of each of these: 

        //"c.lb t1, 0(sp);"     //x c.lb rd, offset(rs1)
        //"c.lbu t2, 0(sp);"    //x c.lbu rd, offset(rs1)
        //"c.ld t3, 0(sp);"     //x c.ld rd, offset(rs1)
        //"c.ldsp t4, 0(sp);"   //x c.ldsp rd, offset(sp)
        //"c.lh t5, 0(sp);"     //x c.lh rd, offset(rs1)
        //"c.lhu t6, 0(sp);"    //x c.lhu rd, offset(rs1)
        //"c.lw t0, 0(sp);"     //x c.lw rd, offset(rs1)
        //"c.lwsp t1, 0(sp);"   //x c.lwsp rd, offset(sp)  
        "fld ft0, 0(sp);"       // fld rd, offset(rs1)
        //"flh ft1, 0(sp);"     //x flh rd, offset(rs1) - Dromajo code: 2675
        "flw ft2, 0(sp);"       // flw rd, offset(rs1)
        "lb t0, 0(sp);"         // lb rd, offset(rs1)
        "lbu t0, 0(sp);"        // lbu rd, offset(rs1)
        "ld t0, 0(sp);"         // ld rd, offset(rs1)
        "lh t0, 0(sp);"         // lh rd, offset(rs1)
        "lhu t0, 0(sp);"        // lhu rd, offset(rs1)
        "lr.d t0, (sp);"        // lr.d rd, (rs1)
        "lr.w t0, (sp);"        // lr.w rd, (rs1)
        "lw t0, 0(sp);"         // lw rd, offset(rs1)
        "lwu t0, 0(sp);"        // lwu rd, offset(rs1)

        // RISC-V defines atomic operations only for words and doublewords
        "lr.w t1, (sp);"
        "sc.w t2, t1, (sp);"

        "lr.d t1, (sp);"
        "sc.d t2, t1, (sp);"

        // Store operations
        // FIXME: should be one of each of these:

        //"c.sb t0, 0(sp);"    //x c.sb rs2, offset(rs1)
        //"c.sd t0, 0(sp);"    //x c.sd rs2, offset(rs1)
        //"c.sh t0, 0(sp);"    //x c.sh rs2, offset(rs1)
        //"c.sw t0, 0(sp);"    //x c.sw rs2, offset(rs1)
        "fsd ft0, 0(sp);"      // fsd rs2, offset(rs1)
        //"fsh ft1, 0(sp);"    //x fsh rs2, offset(rs1) - Dromajo code: 2675
        "fsw ft2, 0(sp);"      // fsw rs2, offset(rs1)
        "sb t0, 0(sp);"        // sb rs2, offset(rs1)
        "sc.d t1, t0, (sp);"   // sc.d rd, rs2, (rs1)
        "sc.w t1, t0, (sp);"   // sc.w rd, rs2, (rs1)
        "sd t0, 0(sp);"        // sd rs2, offset(rs1)
        "sh t0, 0(sp);"        // sh rs2, offset(rs1)
        "sw t0, 4(sp);"        // sw rs2, offset(rs1)
        :
        :
        : "t0", "t1", "t2", "t3", "t4", "t5", "t6", "ft0", "ft1", "ft2"
    );

    STOP_TRACE;
    return 0;
}