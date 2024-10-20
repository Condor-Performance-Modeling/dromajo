# Andes V5 Performance Extension

This is a partial list of AndeStar instructions. Those found in Coremark and Dhrystone.

# Table of Contents
Those marked with N/A are not available in this version of the document, but are planned.

- [ADDIGP](#ADDIGP)
- [BBC](#BBC)
- [BBS](#BBS)
- [BEQC](#BEQC)
- [BFOS](#BFOS)
- [BFOZ](#BFOZ)
- [BNEC](#BNEC)
- [LDGP](#LDGP)
- [LEA.b.ze](#LEA.b.ze)
- [LEA.d](#LEA.d)
- [LEA.d.ze](#LEA.d.ze)
- [LEA.h](#LEA.h)
- [LEA.h.ze](#LEA.h.ze)
- [LEA.w](#LEA.w)
- [LEA.w.ze N/A](#LEA.w.ze)
- [LBGP](#LBGP)
- [LBUGP](#LBUGP)
- [LHGP](#LHGP)
- [LHUGP](#LHUGP)
- [LWGP](#LWGP)
- [LWUGP](#LWUGP)
- [SBGP](#SBGP)
- [SDGP](#SDGP)
- [SHGP](#SHGP)
- [SWGP (FIXME:DETAILS ARE MISSING)](#SWGP)

# Syntax summaries
- Syntax:  ADDIGP Rd, imm[17:0] 
- Syntax:  BBC Rs1, #cimm[4:0], #imm[10:1]  (for RV32) 
- Syntax:  BBC Rs1, #cimm[5:0], #imm[10:1]  (for RV64) 
- Syntax:  BBS Rs1, #cimm[4:0], #imm[10:1]  (for RV32) 
- Syntax:  BBS Rs1, #cimm[5:0], #imm[10:1]  (for RV64)
- Syntax:  BEQC Rs1, #cimm[6:0], #imm[10:1]
- Syntax:  BFOS Rd, Rs1, #msb[4:0], #lsb[4:0]  (for RV32)
- Syntax:  BFOS Rd, Rs1, #msb[5:0], #lsb[5:0]  (for RV64) 
- Syntax:  BFOZ Rd, Rs1, #msb[4:0], #lsb[4:0]  (for RV32) 
- Syntax:  BFOZ Rd, Rs1, #msb[5:0], #lsb[5:0]  (for RV64)
- Syntax:  BNEC Rs1, #cimm[6:0], #imm[10:1] 
- Syntax:  LDGP Rd, [+ (imm[19:3] <<3)]
- Syntax:  LEA.b.ze Rd, Rs1, Rs2 
- Syntax:  LEA.d Rd, Rs1, Rs2 
- Syntax:  LEA.h Rd, Rs1, Rs2
- Syntax:  LEA.h.ze Rd, Rs1, Rs2 
- Syntax:  LEA.w Rd, Rs1, Rs2 
- Syntax:  LEA.w.ze Rd, Rs1, Rs2
- Syntax:  LBGP Rd, [+ imm[17:0]] 
- Syntax:  LBUGP Rd, [+ imm[17:0]] 
- Syntax:  LHGP Rd, [+ (imm[17:1] << 1)] 
- Syntax:  LHUGP Rd, [+ (imm[17:1] << 1)] 
- Syntax:  LWGP Rd, [+ (imm[18:2] <<2)]
- Syntax:  LWUGP Rd, [+ (imm[18:2] <<2)]
- Syntax:  SBGP Rs2, [+ imm[17:0]] 
- Syntax:  SHGP Rs2, [+ (imm[17:1] << 1)] 
- Syntax:  SDGP Rs2, [+ (imm[19:3] << 3)]
- Syntax:  SWGP Rs2, [+ (imm[18:2] << 2)]

# Instructions found in Coremark and Dhrystone

-----------------------------
## ADDIGP

**Syntax**:  `ADDIGP Rd, imm[17:0]`

### P-Code
```
Rd = x3 + SE(imm[17:0]);
```

### Format
```
| 31-30 | 29-21     | 20    | 19-17      | 16-15      | 14-12 | 11-7   | 6-0      |
|-------|-----------|-------|------------|------------|-------|--------|----------|
| imm17 | imm[10:1] | imm11 | imm[14:12] | imm[16:15] | imm0  | ADDIGP | Custom-0 |
|       |           |       |            |            |       |   01   | 0001011  |
|-------|-----------|-------|------------|------------|-------|--------|----------|
```

### Description
`ADDIGP` adds the contents of the implied GP (x3) register with a sign-extended 18-bit immediate constant and writes the result to Rd.

**Arch**: RV32 and RV64

-----------------------------

## BBC (Branch on Test Bit is Clear/Zero)

**Syntax RV32**: `BBC Rs1, #cimm[4:0], #imm[10:1]`

**Syntax RV64**: `BBC Rs1, #cimm[5:0], #imm[10:1]`

### P-Code
```
RV32:
if (Rs1[cimm[4:0]] == 0) {
  tPC = PC + SE(CONCAT(imm[10:1], 0[0]));
  PC = tPC;
}

RV64:
if (Rs1[cimm[5:0]] == 0) {
  tPC = PC + SE(CONCAT(imm[10:1], 0[0]));
  PC = tPC;
}
```

### Format
```
RV32
| 31      | 30  | 29-25    | 24-20     | 19-15 | 14-12 | 11-8     | 7       | 6-0      |
|---------|-----|----------|-----------|-------|-------|----------|---------|----------|
| imm[10] | BBC | imm[9:5] | cimm[4:0] | Rs1   | BTBx  | imm[4:1] | 0       | Custom-2 |
|         |  0  |          |           |       | 111   |          |         | 1011011  |
|---------|-----|----------|-----------|-------|-------|----------|---------|----------|

RV64
| 31      | 30  | 29-25    | 24-20     | 19-15 | 14-12 | 11-8     | 7       | 6-0      |
|---------|-----|----------|-----------|-------|-------|----------|---------|----------|
| imm[10] | BBC | imm[9:5] | cimm[4:0] | Rs1   | BTBx  | imm[4:1] | cimm[5] | Custom-2 |
|         |  0  |          |           |       | 111   |          |         | 1011011  |
|---------|-----|----------|-----------|-------|-------|----------|---------|----------|

```

### Description
BBC (Branch on Test Bit is Clear/Zero) instruction branches if the bit specified by cimm is 0 in the register Rs1.

**Arch**: RV32 and RV64

---

# BBS 

**Syntax RV32**: `BBS Rs1, #cimm[4:0], #imm[10:1]`

**Syntax RV64**: `BBS Rs1, #cimm[5:0], #imm[10:1]`

### P-Code
```
RV32:
if (Rs1[cimm[4:0]] != 0) {
  tPC = PC + SE(CONCAT(imm[10:1], 0[0]));
  PC = tPC;
}

RV64:
if (Rs1[cimm[5:0]] != 0) {
  tPC = PC + SE(CONCAT(imm[10:1], 0[0]));
  PC = tPC;
}
```
### Format
```
RV32
| 31      | 30  | 29-25    | 24-20     | 19-15 | 14-12 | 11-8     | 7       | 6-0      |
|---------|-----|----------|-----------|-------|-------|----------|---------|----------|
| imm[10] | BBS | imm[9:5] | cimm[4:0] | Rs1   | BTBx  | imm[4:1] |  0      | Custom-2 |
|         |  1  |          |           |       | 111   |          |         | 1011011  |
|---------|-----|----------|-----------|-------|-------|----------|---------|----------|

RV64
| 31      | 30  | 29-25    | 24-20     | 19-15 | 14-12 | 11-8     | 7       | 6-0      |
|---------|-----|----------|-----------|-------|-------|----------|---------|----------|
| imm[10] | BBS | imm[9:5] | cimm[4:0] | Rs1   | BTBx  | imm[4:1] | cimm[5] | Custom-2 |
|         |  1  |          |           |       | 111   |          |         | 1011011  |
|---------|-----|----------|-----------|-------|-------|----------|---------|----------|
```

### Description
The `BBS` (Branch on Test Bit is Set/Not Zero) instruction branches if the bit specified by cimm is not 0 in the register Rs1.

**Arch**: RV32 and RV64

---

# BEQC

**Syntax**: `BEQC Rs1, #cimm[6:0], #imm[10:1]`

### P-Code
```
if (Rs1 == ZE(cimm[6:0])) {
  tPC = PC + SE(CONCAT(imm[10:1], 0[0]));
  PC = tPC;
}
```

### Format
```
| 31      | 30      | 29-25    | 24-20     | 19-15 | 14-12 | 11-8     | 7       | 6-0      |
|---------|---------|----------|-----------|-------|-------|----------|---------|----------|
| imm[10] | cimm[6] | imm[9:5] | cimm[4:0] | Rs1   | BEQC  | imm[4:1] | cimm[5] | Custom-2 |
|         |         |          |           |       |  101  |          |         | 1011011  |
|---------|---------|----------|-----------|-------|-------|----------|---------|----------|
```
### Description
The `BEQC` (Branch on Equal to Constant) instruction branches if Rs1 is equal to a zero-extended constant cimm.

**Arch**: RV32 and RV64

---

# BFOS (Sign-extended Bit Field Operation)

**Syntax RV32**: `BFOS Rd, Rs1, #msb[4:0], #lsb[4:0]`

**Syntax RV32**: `BFOS Rd, Rs1, #msb[5:0], #lsb[5:0]`

### P-Code

```
RV32:

LSB = lsb[4:0]
MSB = msb[4:0]
lsbp1 = lsb[4:0] + 1
msbm1 = msb[4:0] - 1
lsbm1 = lsb[4:0] - 1

if (MSB == 0) {
  Rd[LSB] = Rs1[0];
  if (LSB < 31) {
    Rd[31:lsbp1] = REPEAT(Rs1[0]);
  }
  if (LSB > 0) {
    Rd[lsbm1:0] = 0;
  }
} else if (MSB < LSB) {
  lenm1 = LSB - MSB;
  Rd[LSB:MSB] = Rs1[lenm1:0];
  if (LSB < 31) {
    Rd[31:lsbp1] = REPEAT(Rs1[lenm1]);
  }
  Rd[msbm1:0] = 0;
} else { // MSB >= LSB
  lenm1 = MSB - LSB;
  Rd[lenm1:0] = Rs1[MSB:LSB];
  Rd[31:(lenm1 + 1)] = REPEAT(Rs1[MSB]);
}
```
```
RV64:
LSB = lsb[5:0]
MSB = msb[5:0]
lsbp1 = lsb[5:0] + 1
msbm1 = msb[5:0] - 1
lsbm1 = lsb[5:0] - 1

if (MSB == 0) {
  Rd[LSB] = Rs1[0];
  if (LSB < 63) {
    Rd[63:lsbp1] = REPEAT(Rs1[0]);
  }
  if (LSB > 0) {
    Rd[lsbm1:0] = 0;
  }
} else if (MSB < LSB) {
  lenm1 = LSB - MSB;
  Rd[LSB:MSB] = Rs1[lenm1:0];
  if (LSB < 63) {
    Rd[63:lsbp1] = REPEAT(Rs1[lenm1]);
  }
  Rd[msbm1:0] = 0;
} else { // MSB >= LSB
  lenm1 = MSB - LSB;
  Rd[lenm1:0] = Rs1[MSB:LSB];
  Rd[63:(lenm1 + 1)] = REPEAT(Rs1[MSB]);
}
```
### Format
```
RV32
| 31 | 30-26    | 25 | 24-20    | 19-15 | 14-12 | 11-7 | 6-0      |
|----|----------|----|----------|-------|-------|------|----------|
| 0  | msb[4:0] | 0  | lsb[4:0] | Rs1   | BFOS  | Rd   | Custom-2 |
|    |          |    |          |       | 011   |      | 1011011  |
|----|----------|----|----------|-------|-------|------|----------|

RV64
| 31-26         | 25-20         | 19-15 | 14-12 | 11-7 | 6-0      |
|---------------|---------------|-------|-------|------|----------|
|  msb[5:0]     |    lsb[5:0]   | Rs1   | BFOS  | Rd   | Custom-2 |
|               |               |       | 011   | Rd   | 1011011  |
|----|----------|----|----------|-------|-------|------|----------|
```

### Description
The `BFOS` (Sign-extended Bit Field Operation) instruction performs bit field operations, extracting or inserting a sign-extended bit field into Rd.

This instruction contains three different bit-field operations. If msb[5:0] is 0, the 
first operation is performed. If msb[5:0]<lsb[5:0], the second operation is performed. If 
msb[5:0]>=lsb[5:0], the third operation is performed. The first and second operations are sign
extended zero-tailed bit-field insert operations. The third operation is a sign-extended bit-field 
extract operation. 

**Arch**: RV32 and RV64

---

# BFOZ 

**Syntax RV32**: `BFOZ Rd, Rs1, #msb[4:0], #lsb[4:0]`

**Syntax RV64**: `BFOZ Rd, Rs1, #msb[5:0], #lsb[5:0]`

### P-Code
```
RV32:
LSB = lsb[4:0]
MSB = msb[4:0]
lsbp1 = lsb[4:0] + 1
msbm1 = msb[4:0] - 1
lsbm1 = lsb[4:0] - 1

if (MSB == 0) {
  Rd[LSB] = Rs1[0];
  if (LSB < 31) {
    Rd[31:lsbp1] = 0;
  }
  if (LSB > 0) {
    Rd[lsbm1:0] = 0;
  }
} else if (MSB < LSB) {
  lenm1 = LSB - MSB;
  Rd[LSB:MSB] = Rs1[lenm1:0];
  if (LSB < 31) {
    Rd[31:lsbp1] = 0;
  }
  Rd[msbm1:0] = 0;
} else { // MSB >= LSB
  lenm1 = MSB - LSB;
  Rd[lenm1:0] = Rs1[MSB:LSB];
  Rd[31:(lenm1 + 1)] = 0;
}
```
```
RV64:
LSB = lsb[5:0]
MSB = msb[5:0]
lsbp1 = lsb[5:0] + 1
msbm1 = msb[5:0] - 1
lsbm1 = lsb[5:0] - 1

if (MSB == 0) {
  Rd[LSB] = Rs1[0];
  if (LSB < 63) {
    Rd[63:lsbp1] = 0;
  }
  if (LSB > 0) {
    Rd[lsbm1:0] = 0;
  }
} else if (MSB < LSB) {
  lenm1 = LSB - MSB;
  Rd[LSB:MSB] = Rs1[lenm1:0];
  if (LSB < 63) {
    Rd[63:lsbp1] = 0;
  }
  Rd[msbm1:0] = 0;
} else { // MSB >= LSB
  lenm1 = MSB - LSB;
  Rd[lenm1:0] = Rs1[MSB:LSB];
  Rd[63:(lenm1 + 1)] = 0;
}

```
### Format
```
RV32
| 31 | 30-26    | 25 | 24-20    | 19-15 | 14-12 | 11-7 | 6-0      |
|----|----------|----|----------|-------|-------|------|----------|
| 0  | msb[4:0] | 0  | lsb[4:0] | Rs1   | BFOZ  | Rd   | Custom-2 |
|    |          |    |          |       |  010  |      | 1011011  |
|----|----------|----|----------|-------|-------|------|----------|

RV64
| 31-26         | 25-20         | 19-15 | 14-12 | 11-7 | 6-0      |
|---------------|---------------|-------|-------|------|----------|
|   msb[5:0]    |   lsb[5:0]    | Rs1   | BFOZ  | Rd   | Custom-2 |
|               |               |       |  010  |      | 1011011  |
|---------------|---------------|-------|-------|------|----------|
```
### Description
The `BFOZ` (Zero-extended Bit Field Operation) instruction performs bit field operations, extracting or inserting a zero-extended bit field into Rd.

This instruction contains three different bit-field operations. If msb[5:0] is 0, the 
first operation is performed. If msb[5:0]<lsb[5:0], the second operation is performed. If 
msb[5:0]>=lsb[5:0], the third operation is performed. The first and second operations are zero
extended zero-tailed bit-field insert operations. The third operation is a zero-extended bit-field 
extract operation. 

**Arch**: RV32 and RV64

---

# BNEC (Branch on Not Equal to Constant)

**Syntax**: `BNEC Rs1, #cimm[6:0], #imm[10:1]`

### P-Code
```
if (Rs1 != ZE(cimm[6:0])) {
  tPC = PC + SE(CONCAT(imm[10:1], 0[0]));
  PC = tPC;
}
```
### Format
```
| 31      | 30-25   | 24-20    | 19-15 | 14-12 | 11-8     | 7       | 6-0     |
|---------|---------|----------|-------|-------|----------|---------|---------|
| imm[10] | cimm[6] | imm[9:5] | Rs1   | BNEC  | imm[4:1] | cimm[5] |Custom-2 |
|         |         |          |       |  110  |          |         | 1011011 |
|---------|---------|----------|-------|-------|----------|---------|---------|
```
### Description
The `BNEC` (Branch on Not Equal to Constant) instruction branches if Rs1 is not equal to a zero-extended constant cimm.

**Arch**: RV32 and RV64
---
# LBGP

**Syntax**: `LBGP Rd, [+ imm[17:0]]`

### P-Code
Rd = x3 + SE(imm[17:0]);

### Format
| 31-30   | 29-21     | 20    | 19-17   | 16-15   | 14-12 | 11-7 | 6-0     |
|---------|-----------|-------|---------|---------|-------|------|---------|
| imm[17] | imm[10:1] | imm11 | imm[14:12] | imm[16:15] | Rd   | Custom-1 |

### Description
`LBGP` loads a byte from memory into a general-purpose register, using an offset based on the GP register (`x3`).

### Arch
RV32 and RV64
```
# LBUGP

**Syntax**: `LBUGP Rd, [+ imm[17:0]]`

### P-Code
Rd = x3 + ZE(imm[17:0]);

### Format
| 31-30   | 29-21     | 20    | 19-17   | 16-15   | 14-12 | 11-7 | 6-0     |
|---------|-----------|-------|---------|---------|-------|------|---------|
| imm[17] | imm[10:1] | imm11 | imm[14:12] | imm[16:15] | Rd   | Custom-1 |

### Description
`LBUGP` loads a byte from memory into a general-purpose register, using a zero-extended offset based on the GP register (`x3`).

### Arch
RV32 and RV64

---
# LHGP

**Syntax**: `LHGP Rd, [+ (imm[17:1] << 1)]`

### P-Code
Rd = x3 + SE(imm[17:1] << 1);

### Format
| 31-30   | 29-21     | 20    | 19-17   | 16-15   | 14-12 | 11-7 | 6-0     |
|---------|-----------|-------|---------|---------|-------|------|---------|
| imm[17] | imm[10:1] | imm11 | imm[14:12] | imm[16:15] | Rd   | Custom-1 |

### Description
`LHGP` loads a half-word from memory into a general-purpose register, using a sign-extended offset based on the GP register (`x3`).

### Arch
RV32 and RV64
---
# LHUGP

**Syntax**: `LHUGP Rd, [+ (imm[17:1] << 1)]`

### P-Code
Rd = x3 + ZE(imm[17:1] << 1);

### Format
| 31-30   | 29-21     | 20    | 19-17   | 16-15   | 14-12 | 11-7 | 6-0     |
|---------|-----------|-------|---------|---------|-------|------|---------|
| imm[17] | imm[10:1] | imm11 | imm[14:12] | imm[16:15] | Rd   | Custom-1 |

### Description
`LHUGP` loads an unsigned half-word from memory into a general-purpose register, using a zero-extended offset based on the GP register (`x3`).

### Arch
RV32 and RV64

---
# LDGP

**Syntax**: `LDGP Rd, [+ (imm[19:3] << 3)]`

### P-Code
Rd = x3 + SE(imm[19:3] << 3);

### Format
```
| 31-30   | 29-21     | 20    | 19-17   | 16-15   | 14-12 | 11-7 | 6-0     |
|---------|-----------|-------|---------|---------|-------|------|---------|
| imm[19] | imm[10:3] | Rs2   | imm[14:12] | imm[16:15] | Rd   | Custom-1 |
```

### Description
`LDGP` loads a double-word from memory into a general-purpose register, using an offset based on the GP register (`x3`).

### Arch
RV64
---

# LEA.h (Load Effective Half-word Address)

**Syntax**: `LEA.h Rd, Rs1, Rs2`

### P-Code
```
Rd = Rs1 + Rs2 * 2;
```
### Format
```
| 31-25   | 24-20 | 19-15 | 14-12 | 11-7 | 6-0      |
|---------|-------|-------|-------|------|----------|
| LEA.h   | Rs2   | Rs1   | func3 | Rd   | Custom-2 |
| 0000101 |       |       | 000   |      | 1011011  |
|---------|-------|-------|-------|------|----------|
```
### Description
The `LEA.h` (Load Effective Half-word Address) instruction adds a base register with a half-word-aligned offset from an offset register and writes the result to Rd.

**Arch**: RV32 and RV64

---

# LEA.w (Load Effective Word Address)

**Syntax**: `LEA.w Rd, Rs1, Rs2`

### P-Code
```
Rd = Rs1 + Rs2 * 4;
```
### Format
```
| 31-25   | 24-20 | 19-15 | 14-12 | 11-7 | 6-0      |
|---------|-------|-------|-------|------|----------|
| LEA.w   | Rs2   | Rs1   | func3 | Rd   | Custom-2 |
| 0000110 |       |       | 000   |      | 1011011  |
|---------|-------|-------|-------|------|----------|
```
### Description
The `LEA.w` (Load Effective Word Address) instruction adds a base register with a word-aligned offset from an offset register and writes the result to Rd.

**Arch**: RV32 and RV64

---

# LWGP 

**Syntax**: `LWGP Rd, [+ (imm[18:2] << 2)]`

### P-Code
```
Rd = x3 + SE(imm[18:2] << 2);
```
### Format
```
| 31-30   | 29-21     | 20      | 19-17      | 16-15      | 14-12 | 11-7 | 6-0      |
|---------|-----------|---------|------------|------------|-------|------|----------|
| imm[18] | imm[10:2] | imm[17] | imm[14:12] | imm[16:15] | LWGP  | Rd   | Custom-1 |
|         |           |         |            |            |  010  |      | 0101011  |
|---------|-----------|---------|------------|------------|-------|------|----------|
```
### Description
The `LWGP` (GP-implied Load Word Signed Immediate) instruction loads a signed word from memory into a general-purpose register based on the offset and GP register (x3).

**Arch**: RV32 and RV64

---
# LWUGP

**Syntax**: `LWUGP Rd, [+ (imm[18:2] << 2)]`

### P-Code
Rd = x3 + ZE(imm[18:2] << 2);

### Format
| 31-30   | 29-21     | 20    | 19-17   | 16-15   | 14-12 | 11-7 | 6-0     |
|---------|-----------|-------|---------|---------|-------|------|---------|
| imm[18] | imm[10:2] | imm11 | imm[14:12] | imm[16:15] | Rd   | Custom-1 |

### Description
`LWUGP` loads an unsigned word from memory into a general-purpose register, using a zero-extended offset based on the GP register (`x3`).

### Arch
RV32 and RV64

-------------------------------------------------------------------------------------------------
# SBGP

**Syntax**: `SBGP Rs2, [+ imm[17:0]]`

### P-Code
Memory[x3 + SE(imm[17:0])] = Rs2;

### Format
| 31-30   | 29-21     | 20    | 19-17   | 16-15   | 14-12 | 11-7 | 6-0     |
|---------|-----------|-------|---------|---------|-------|------|---------|
| imm[17] | imm[10:1] | imm11 | imm[14:12] | imm[16:15] | Rs2  | Custom-1 |

### Description
`SBGP` stores a byte from the register `Rs2` to memory at an address calculated by adding a sign-extended offset to the GP register (`x3`).

### Arch
RV32 and RV64

-------------------------------------------------------------------------------------------------
# SDGP 

**Syntax**: `SDGP Rs2, [+ (imm[19:3] << 3)]`

### P-Code
```
Memory[x3 + SE(imm[19:3] << 3)] = Rs2;
```
### Format
```
| 31    | 30-25   | 24-20 | 19-17    | 16-15    | 14-12 | 11-10    | 9-8      | 7     | 6-0      |
|-------|---------|-------|----------|----------|-------|----------|----------|-------|----------|
| i[19] | i[10:5] | Rs2   | i[14:12] | i[16:15] | SDGP  | i[4:3]   | i[18:17] | i[11] | Custom-1 |
|       |         |       |          |          |  111  |          |          |       | 0101011  |
|-------|---------|-------|----------|----------|-------|----------|----------|-------|----------|
```
### Description
The `SDGP` (GP-implied Store Double-word Immediate) instruction stores a double-word into memory from Rs2 at an address relative to the GP register (x3).

**Arch**: RV64

-------------------------------------------------------------------------------------------------
# SHGP

**Syntax**: `SHGP Rs2, [+ (imm[17:1] << 1)]`

### P-Code
Memory[x3 + SE(imm[17:1] << 1)] = Rs2;

### Format
| 31-30   | 29-21     | 20    | 19-17   | 16-15   | 14-12 | 11-7 | 6-0     |
|---------|-----------|-------|---------|---------|-------|------|---------|
| imm[17] | imm[10:1] | imm11 | imm[14:12] | imm[16:15] | Rs2  | Custom-1 |

### Description
`SHGP` stores a half-word from the register `Rs2` to memory at an address calculated by adding a sign-extended offset to the GP register (`x3`).

### Arch
RV32 and RV64



