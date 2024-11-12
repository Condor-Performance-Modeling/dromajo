#include "trace_macros.h"
#include <stdint.h>
// For linux define these locally, for bare metal
// they are present in linker control and crt.S
#ifdef __linux
#include <stdio.h>
volatile uint64_t tohost   __attribute__((used));
volatile uint64_t fromhost __attribute__((used));
volatile char     conio    __attribute__((used));
void _pass() __attribute__((used,noinline));
void _pass() {}
#else
extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;
extern volatile char     conio;
extern void _pass() __attribute__((used,noinline));
#endif
#define BEGIN 1
#define END   0
void decoder();
void catch_all();
void zfa();
// -----------------------------------------------------------------
// -----------------------------------------------------------------
int main(int ac,char **av)
{
  (void)ac;
  (void)av;
  START_TRACE
  decoder();
  STOP_TRACE
  tohost = 1;
  return 0;
}
// -----------------------------------------------------------------
// -----------------------------------------------------------------
void decoder()
{
  catch_all();
  zfa();
}

void zfa()
{
  
}
void catch_all()
{
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");

    asm volatile ("add	a5,a5,a3");
    asm volatile ("addi	sp,sp,-16");
    asm volatile ("addiw	a5,a5,1");
    asm volatile ("addw	a4,t3,a2");
    asm volatile ("amoadd.d	zero,x0,(x0)");
    asm volatile ("amoadd.d.aq	zero,x0,(x0)");
    asm volatile ("amoadd.w	a5,x0,(x0)");
    asm volatile ("amoadd.w.aq	a5,a4,(s0)");
    asm volatile ("amoadd.w.aqrl	a5,x0,(x0)");
    asm volatile ("amoand.w.aq	zero,x0,(x0)");
    asm volatile ("amomaxu.d.aq	x0,x0,(x0)");
    asm volatile ("amomaxu.w.aq	x0,x0,(x0)");
    asm volatile ("amoor.w	zero,x0,(x0)");
    asm volatile ("amoor.w.aq	a5,x0,(x0)");
    asm volatile ("amoswap.d.aq	a0,x0,(x0)");
    asm volatile ("amoswap.d.aqrl	a1,x0,(x0)");
    asm volatile ("amoswap.w	x0,x0,(x0)");
    asm volatile ("amoswap.w.aq	x0,x0,(x0)");
    asm volatile ("amoswap.w.rl	x0,x0,(x0)");
    asm volatile ("and	a3,a1,a2");
    asm volatile ("andi	sp,sp,-16");
    asm volatile ("auipc	a5,0x91");
    asm volatile ("bseti	s2,zero,0x27");
    asm volatile ("c.nop	2");
    asm volatile ("c.slli	zero,0x2");
    asm volatile ("c.slli64	zero");
    //FALL THROUGH asm volatile ("csrr	a0,vlenb");
    asm volatile ("csrs	fflags,a4");
    asm volatile ("csrsi	fflags,16");
    asm volatile ("div	a0,a0,a7");
    asm volatile ("divu	s3,s3,s7");
    asm volatile ("divuw	a3,a3,a4");
    asm volatile ("divw	a5,a5,a3");
    //ILLEGAL asm volatile ("ebreak");
    asm volatile ("fabs.d	fa5,fa0");
    asm volatile ("fadd.d	ft1,fa5,fa4");
    asm volatile ("fadd.s	ft3,ft2,fa2");
    asm volatile ("fcvt.d.s	ft2,ft1");
    asm volatile ("fcvt.d.w	fa0,s3");
    asm volatile ("fcvt.s.d	ft2,ft0");
    asm volatile ("fcvt.s.w	ft2,t0");
    asm volatile ("fcvt.w.d	t4,ft0,rtz");
    asm volatile ("fcvt.w.s	a0,fa2,rtz");
    asm volatile ("fdiv.d	ft0,fa4,fa3");
    asm volatile ("fdiv.s	ft6,ft3,fs0");
    asm volatile ("fence	rw,rw");
    asm volatile ("feq.d	a5,fa0,fa0");
    asm volatile ("fld	fs0,568(sp)");
    asm volatile ("fle.s	t1,ft6,ft5");
    //FALL THROUGH asm volatile ("fli.d	fs1,0x1p-1");
    //FALL THROUGH ILLEGAL asm volatile ("fli.s	ft5,0x1p+0");

    asm volatile ("flt.d	a5,fa4,fa5");
    asm volatile ("flt.s	a2,ft5,fa7");
    asm volatile ("flw	fs3,0(x0) # a28f0 <__TMC_END__+0x30>");
    asm volatile ("fmadd.d	ft0,fa5,fs0,fs1");
    asm volatile ("fmadd.s	ft4,fa4,fs2,ft3");
    asm volatile ("fmax.s	fa7,ft3,fa6");
    asm volatile ("fmul.d	ft3,ft2,ft2");
    asm volatile ("fmul.s	ft3,ft2,fs3");
    asm volatile ("fmv.d	fa5,fa0");
    asm volatile ("fmv.d.x	fa1,a2");
    asm volatile ("fmv.s	ft6,fa2");
    asm volatile ("fmv.w.x	fa5,zero");
    asm volatile ("fmv.x.d	a5,fa0");
    asm volatile ("fneg.d	fa5,fa0");
    asm volatile ("fneg.s	fa0,fa0");
    asm volatile ("fnmsub.d	fa3,fa5,fa3,fa0");
    asm volatile ("frflags	a4");
    //FALL THROUGH asm volatile ("fround.s	ft7,ft6,rtz");
    asm volatile ("frrm	a1");
    asm volatile ("fsd	fs0,568(sp)");
    asm volatile ("fsflags	a4");
    asm volatile ("fsqrt.s	fa0,fa0");
    asm volatile ("fsub.d	fa4,fa4,ft1");
    asm volatile ("fsub.s	fa0,ft6,ft7");
    asm volatile ("fsw	fs0,312(s11)");
    asm volatile ("lb	a0,1(x0)");
    asm volatile ("lbu	s8,0(x0)");
    asm volatile ("ld	a5,0(x0) ");
    asm volatile ("lh	a2,0(x0)");
    asm volatile ("lhu	t3,0(x0) # a2bca <way+0xaa>");
    asm volatile ("li	a0,8");
    asm volatile ("lr.d	a4,(a3)");
    asm volatile ("lr.d.aq	a5,(a0)");
    asm volatile ("lr.d.aqrl	a3,(a4)");
    asm volatile ("lr.w	a3,(a5)");
    asm volatile ("lr.w.aq	a5,(s0)");
    asm volatile ("lui	a5,0xb8b1b");
    asm volatile ("lw	a5,0(x0) # a4784 <lock+0x4>");
    asm volatile ("lwu	a5,0(s4)");
    asm volatile ("max	s2,s2,zero");
    asm volatile ("min	t2,s5,a2");
    asm volatile ("mul	a0,t0,a3");
    asm volatile ("mulh	a4,a1,a4");
    asm volatile ("mulhu	a5,a0,a1");
    asm volatile ("mulw	s6,s3,s5");
    asm volatile ("mv	s0,a0");
    asm volatile ("neg	a5,a5");
    asm volatile ("negw	a1,s1");
    asm volatile ("nop");
    asm volatile ("not	a5,a5");
    asm volatile ("or	a7,a4,a6");
    asm volatile ("ori	a3,a5,1");
    asm volatile ("remu	a0,a0,a7");
    asm volatile ("remuw	a4,a4,a7");
    asm volatile ("remw	a3,a3,a5");
    asm volatile ("sb	a5,0(x0) # a3ef0 <in_shutdown>");
    asm volatile ("sc.d	a2,a3,(a0)");
    asm volatile ("sc.d.rl	a2,s0,(a4)");
    asm volatile ("sc.w	a3,a4,(s0)");
    asm volatile ("sc.w.rl	a1,a4,(s0)");
    asm volatile ("mv	sp,x0");
    asm volatile ("sd	ra,8(sp)");
    asm volatile ("seqz	s7,a2");
    asm volatile ("sext.h	s0,a5");
    asm volatile ("sext.w	a5,a5");
    asm volatile ("sgtz	a4,a4");
    asm volatile ("sh	s8,0(s9)");
    asm volatile ("sh1add	s9,a7,a2");
    asm volatile ("sh2add	t6,a1,a0");
    asm volatile ("sh3add	a4,t5,t3");
    asm volatile ("sll	a6,a6,a7");
    asm volatile ("slli	a5,a5,0x20");
    asm volatile ("slliw	a0,t0,0x3");
    asm volatile ("sllw	a6,a0,a3");
    asm volatile ("slt	a0,a0,a7");
    asm volatile ("slti	a7,a4,0");
    asm volatile ("sltiu	a1,a1,2");
    asm volatile ("sltu	a5,a5,a4");
    asm volatile ("snez	a0,t0");
    asm volatile ("sra	a0,a0,a5");
    asm volatile ("srai	t1,a0,0x2d");
    asm volatile ("sraiw	t2,t0,0x1f");
    asm volatile ("sraw	t4,t3,a7");
    asm volatile ("srl	a0,a0,a5");
    asm volatile ("srli	a5,a4,0x1c");
    asm volatile ("srliw	t4,t3,0x18");
    asm volatile ("srlw	a3,a5,a3");
    asm volatile ("sub	a2,a3,a5");
    asm volatile ("subw	t2,t1,t2");
    asm volatile ("sw	a5,4(x0)");
    asm volatile ("xori	a0,a0,1");
    asm volatile ("zext.b	s9,t5");
    asm volatile ("zext.h	t1,a7");
    asm volatile ("zext.w	a4,a1");
}

