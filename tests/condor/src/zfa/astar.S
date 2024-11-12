

addi  sp,sp,-16
li  a0,8
sd  ra,8(sp)
auipc a5,0x91
ld  a5,-1018(a5)
addi  a5,a5,16
sd  a5,0(a0)
auipc a2,0x91
ld  a2,-1502(a2)
auipc a1,0x91
ld  a1,-1830(a1)

