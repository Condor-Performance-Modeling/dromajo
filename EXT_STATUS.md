
# Overview

Implemented - functionality present in dromajo decoder
Tested - functionality has passing test in dromajo regression
Mavis - instruction has presentation in Mavis format

EXT - which extension
ISA = RV32 and/or RV64
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

```
Implemented Tested  Mavis   Mnemonic        EXT   ISA 
y           y       -       ADDIGP          A*    ?      
y           -       -       BBC             A*    ?      
y           -       -       BBS             A*    ?      
y           -       -       BEQC            A*    ?      
y           -       -       BFOS            A*    ?      
y           -       -       BFOZ            A*    ?      
y           -       -       BNEC            A*    ?      
y           -       -       LBGP            A*    ?
y           -       -       LBUGP           A*    ?
y           -       -       LDGP            A*    ?
y           -       -       LEA.b.ze        A*    ?      
y           -       -       LEA.d           A*    ?      
y           -       -       LEA.h           A*    ?      
y           -       -       LEA.h.ze        A*    ?      
y           -       -       LEA.w           A*    ?      
y           -       -       LEA.w.ze        A*    ?      
y           -       -       LHGP            A*    ?
y           -       -       LHUGP           A*    ?
y           -       -       LWGP            A*    ?      
y           -       -       LWUGP           A*    ?
y           -       -       SBGP            A*    ?
y           -       -       SDGP            A*    ?      
y           -       -       SHGP            A*    ?
y           -       -       SWGP            A*    ?      

```
