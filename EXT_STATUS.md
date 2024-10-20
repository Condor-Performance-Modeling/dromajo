
# Overview

Implemented - functionality present in dromajo decoder
Tested - functionality has passing test in dromajo regression
Mavis - instruction has presentation in Mavis format

EXT - which extension
ISA = RV32 and/or RV64
Mavis = j means json only

## Instruction status
```
Implemented Tested  Mavis   Mnemonic        EXT   ISA 
                                                              
y           -       y       add.uw          zba   RV64
y           -       y       andn            zbb   RV32 RV64   
y           -       y       bclr            zbs   RV32 RV64
y           -       y       bclri           zbs   RV32 RV64
y           -       y       bext            zbs   RV32 RV64
y           -       y       bexti           zbs   RV32 RV64
y           -       y       binv            zbs   RV32 RV64
y           -       y       binvi           zbs   RV32 RV64
y           -       y       bset            zbs   RV32 RV64
y           -       y       bseti           zbs   RV32 RV64
y           -       y       clmul           zbc   RV32 RV64
y           -       y       clmulh          zbc   RV32 RV64
y           -       y       clmulr          zbc   RV32 RV64   
y           -       y       clz             zbb   RV32 RV64   
y           -       y       clzw            zbb   RV64        
y           -       y       cpop            zbb   RV32 RV64   
y           -       y       cpopw           zbb   RV64        
y           -       y       ctz             zbb   RV32 RV64   
y           -       y       ctzw            zbb   RV64        
y           -       y       max             zbb   RV32 RV64   
y           -       y       maxu            zbb   RV32 RV64   
y           -       y       min             zbb   RV32 RV64   
y           -       y       minu            zbb   RV32 RV64   
-           -       y       orc.b           ???   RV32 RV64
y           -       y       orn             zbb   RV32 RV64   
-           -       y       rev8            ???   RV32 RV64 
-           -       y       rol             ???   RV32 RV64
-           -       y       rolw            ???   RV64
-           -       y       ror             ???   RV32 RV64
-           -       y       rori            ???   RV32 RV64
-           -       y       roriw           ???   RV64
-           -       y       rorw            ???   RV64
y           -       y       sext.b          zbb   RV32 RV64   
y           -       y       sext.h          zbb   RV32 RV64   
y           -       y       sh1add          zba   RV32 RV64   
y           -       y       sh1add.uw       zba   RV64
y           -       y       sh2add          zba   RV32 RV64
y           -       y       sh2add.uw       zba   RV64
y           -       y       sh3add          zba   RV32 RV64
y           -       y       sh3add.uw       zba   RV64
y           -       y       slli.uw         zba   RV64
y           -       y       xnor            zbb   RV32 RV64   
y           -       y       zext.h          zbb   RV32 RV64   

Also - had to reimplement this, needs verified again
D  sub
```

# Andes V5 Performance extension
```
Implemented Tested  Mavis   Mnemonic        EXT   ISA 
y           y       y       ADDIGP          A*    RV32 RV64
y           -       j       BBC             A*    RV32 RV64
y           -       j       BBS             A*    RV32 RV64
y           -       j       BEQC            A*    RV32 RV64
y           -       j       BNEC            A*    RV32 RV64
y           -       j       BFOS            A*    RV32 RV64
y           -       j       BFOZ            A*    RV32 RV64
y           -       j       LEA.h           A*    RV32 RV64
y           -       j       LEA.w           A*    RV32 RV64
y           -       j       LEA.d           A*    RV32 RV64
y           -       j       LEA.b.ze        A*         RV64
y           -       j       LEA.h.ze        A*         RV64
y           -       j       LEA.w.ze        A*         RV64
-           -       j       LEA.d.ze        A*         RV64 
y           -       j       LBGP            A*    RV32 RV64
y           -       j       LBUGP           A*    RV32 RV64
y           -       j       LHGP            A*    RV32 RV64
y           -       j       LHUGP           A*    RV32 RV64
y           -       y       LWGP            A*    RV32 RV64
y           -       j       LWUGP           A*         RV64
y           -       j       LDGP            A*         RV64
y           -       j       SBGP            A*    RV32 RV64
y           -       y       SDGP            A*         RV64
y           -       j       SHGP            A*    RV32 RV64
y           -       j       SWGP            A*    RV32 RV64
-           -       -       FFB             A*    RV32 RV64
-           -       -       FFZMISM         A*    RV32 RV64
-           -       -       FFMISM          A*    RV32 RV64
-           -       -       FLMISM          A*    RV32 RV64
```

# Andes Code dense extension
```
Implemented Tested  Mavis   Mnemonic        EXT   ISA 
-           -       -       EXEC.IT         A*    RV32 RV64
-           -       -       NEXEC.IT        A*    RV32 RV64
```

# Andes Vector load extension
```
Implemented Tested  Mavis   Mnemonic        EXT   ISA 
-           -       -       VLN8.V          A*    n/a
-           -       -       VLNU8.V         A*    n/a
```

# Andes V5 scalar bloat16 conversion
```
Implemented Tested  Mavis   Mnemonic        EXT   ISA 
-           -       -       FCVT.S.BF16     A*    n/a
-           -       -       FCVT.S.BF16.S   A*    n/a
```

# Andes V5 vector bloat16 conversion
```
Implemented Tested  Mavis   Mnemonic        EXT   ISA 
-           -       -       VFWCVT.S.BF16   A*    n/a
-           -       -       VFNCVT.BF16.S   A*    n/a
```

# Andes V5 vector packed FP16 extension
```
Implemented Tested  Mavis   Mnemonic        EXT   ISA 
-           -       -       VFPMADT.VF      A*    n/a
-           -       -       VFPMADB.VF      A*    n/a
```

# Andes V5 vector dot product extension
```
Implemented Tested  Mavis   Mnemonic        EXT   ISA 
-           -       -       VD4DOTS.VV      A*    n/a
-           -       -       VD4DOTU.VV      A*    n/a
-           -       -       VD4DOTSU.VV     A*    n/a
```

# V5 Andes vector small int handling extension
```
Implemented Tested  Mavis   Mnemonic        EXT   ISA 
-           -       -       VLEA4.V         A*    n/a
-           -       -       VFWCVT.F.N.V    A*    n/a
-           -       -       VFWCVT.F.NU.V   A*    n/a
-           -       -       VFWCVT.F.B.V    A*    n/a
-           -       -       VFWCVT.F.BU.V   A*    n/a
```

Format helper
```

3322 2222 2222 1111 1111 11-- ---- ----
1098 7654 3210 9876 5432 1098 7654 3210
---- ---- ---- ---- ---- ---- ---- ----
```

