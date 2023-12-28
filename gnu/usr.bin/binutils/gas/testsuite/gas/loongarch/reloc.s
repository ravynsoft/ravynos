/* Test insn relocs.  */
.text
nop

/* Jump Insns.  */
/* b16.  */
beq  $r4,$r5,%b16(.L1)
bne  $r4,$r5,%b16(.L1)
blt  $r4,$r5,%b16(.L1)
bge  $r4,$r5,%b16(.L1)
bltu $r4,$r5,%b16(.L1)
bgeu $r4,$r5,%b16(.L1)
jirl $r4,$r5,%b16(.L1)

/* b21.  */
beqz $r4,%b21(.L1)
bnez $r4,%b21(.L1)

/* b26.  */
b  %b26(.L1)
bl %b26(.L1)


/* ABS Insns.  */
/* lu12i.w.  */
lu12i.w $r4,%abs_hi20(.L1)

/* ori   */
ori $r4,$r5,%abs_lo12(.L1)

/* lu32i.d.  */
lu32i.d $r4,%abs64_lo20(.L1)

/* lu52i.d.  */
lu52i.d $r5,$r4,%abs64_hi12(.L1)


/* Pcala Insns.  */
/* pcalau12i.  */
pcalau12i $r4,%pc_hi20(.L1)
pcalau12i $r4,%got_pc_hi20(.L1)
pcalau12i $r4,%got_pc_lo12(.L1)
pcalau12i $r4,%ie_pc_hi20(TLSL1)
pcalau12i $r4,%ld_pc_hi20(TLSL1)
pcalau12i $r4,%gd_pc_hi20(TLSL1)

/* addi.w/d ld.b/h/w/d.  */
addi.w $r5,$r4,%pc_lo12(.L1)
addi.d $r5,$r4,%pc_lo12(.L1)
ld.b $r5,$r4,%pc_lo12(.L1)
ld.h $r5,$r4,%pc_lo12(.L1)
ld.w $r5,$r4,%pc_lo12(.L1)
ld.d $r5,$r4,%pc_lo12(.L1)
lu32i.d $r4,%pc64_lo20(.L1)
lu52i.d $r5,$r4,%pc64_lo20(.L1)
lu32i.d $r4,%got64_pc_lo20(.L1)
lu52i.d $r5,$r4,%got64_pc_hi12(.L1)


/* GOT64 Insns.  */
/* lu12i.w.  */
lu12i.w $r4,%got_hi20(.L1)
ori $r4,$r4,%got_lo12(.L1)
lu32i.d $r4,%got64_lo20(.L1)
lu52i.d $r5,$r4,%got64_hi12(.L1)


/* TLS Insns.  */
lu12i.w $r4,%le_hi20(TLSL1)
ori $r5,$r4,%le_lo12(TLSL1)
lu32i.d $r4,%le64_lo20(TLSL1)
lu52i.d $r5,$r4,%le64_hi12(TLSL1)



/* Insns with addend.  */
/* Jump Insns.  */
/* b16.  */
beq  $r4,$r5,%b16(.L1 + 0x8)
bne  $r4,$r5,%b16(.L1 + 0x8)
blt  $r4,$r5,%b16(.L1 + 0x8)
bge  $r4,$r5,%b16(.L1 + 0x8)
bltu $r4,$r5,%b16(.L1 + 0x8)
bgeu $r4,$r5,%b16(.L1 + 0x8)
jirl $r4,$r5,%b16(.L1 + 0x8)

/* b21.  */
beqz $r4,%b21(.L1 + 0x8)
bnez $r4,%b21(.L1 + 0x8)

/* b26.  */
b  %b26(.L1 + 0x8)
bl %b26(.L1 + 0x8)


/* ABS Insns.  */
/* lu12i.w.  */
lu12i.w $r4,%abs_hi20(.L1 + 0x8)

/* ori   */
ori $r4,$r5,%abs_lo12(.L1 + 0x8)

/* lu32i.d.  */
lu32i.d $r4,%abs64_lo20(.L1 + 0x8)

/* lu52i.d.  */
lu52i.d $r5,$r4,%abs64_hi12(.L1 + 0x8)


/* Pcala Insns.  */
/* pcalau12i.  */
pcalau12i $r4,%pc_hi20(.L1 + 0x8)
pcalau12i $r4,%got_pc_hi20(.L1 + 0x8)
pcalau12i $r4,%got_pc_lo12(.L1 + 0x8)
pcalau12i $r4,%ie_pc_hi20(TLSL1 + 0x8)
pcalau12i $r4,%ld_pc_hi20(TLSL1 + 0x8)
pcalau12i $r4,%gd_pc_hi20(TLSL1 + 0x8)

/* addi.w/d ld.b/h/w/d.  */
addi.w $r5,$r4,%pc_lo12(.L1 + 0x8)
addi.d $r5,$r4,%pc_lo12(.L1 + 0x8)
ld.b $r5,$r4,%pc_lo12(.L1 + 0x8)
ld.h $r5,$r4,%pc_lo12(.L1 + 0x8)
ld.w $r5,$r4,%pc_lo12(.L1 + 0x8)
ld.d $r5,$r4,%pc_lo12(.L1 + 0x8)
lu32i.d $r4,%pc64_lo20(.L1 + 0x8)
lu52i.d $r5,$r4,%pc64_lo20(.L1 + 0x8)
lu32i.d $r4,%got64_pc_lo20(.L1 + 0x8)
lu52i.d $r5,$r4,%got64_pc_hi12(.L1 + 0x8)


/* GOT64 Insns.  */
/* lu12i.w.  */
lu12i.w $r4,%got_hi20(.L1 + 0x8)
ori $r4,$r4,%got_lo12(.L1 + 0x8)
lu32i.d $r4,%got64_lo20(.L1 + 0x8)
lu52i.d $r5,$r4,%got64_hi12(.L1 + 0x8)


/* TLS Insns.  */
lu12i.w $r4,%le_hi20(TLSL1 + 0x8)
ori $r5,$r4,%le_lo12(TLSL1 + 0x8)
lu32i.d $r4,%le64_lo20(TLSL1 + 0x8)
lu52i.d $r5,$r4,%le64_hi12(TLSL1 + 0x8)
