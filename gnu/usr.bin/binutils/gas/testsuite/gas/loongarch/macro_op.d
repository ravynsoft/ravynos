#as:
#objdump: -dr
#skip: loongarch32-*-*

.*:[    ]+file format .*


Disassembly of section .text:

00000000.* <.text>:
[ 	]+0:[ 	]+00150004[ 	]+move[ 	]+\$a0,[ 	]+\$zero
[ 	]+4:[ 	]+02bffc04[ 	]+li\.w[ 	]+\$a0,[ 	]+-1
[ 	]+8:[ 	]+00150004[ 	]+move[ 	]+\$a0,[ 	]+\$zero
[ 	]+c:[ 	]+02bffc04[ 	]+li\.w[ 	]+\$a0,[ 	]+-1
[ 	]+10:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+10:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+\.L1
[ 	]+10:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+14:[ 	]+28c00084[ 	]+ld\.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+14:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+\.L1
[ 	]+14:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+18:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+18:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+\.L1
[ 	]+18:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+1c:[ 	]+28c00084[ 	]+ld\.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+1c:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+\.L1
[ 	]+1c:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+20:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+20:[ 	]+R_LARCH_PCALA_HI20[ 	]+\.L1
[ 	]+20:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+24:[ 	]+02c00084[ 	]+addi\.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+24:[ 	]+R_LARCH_PCALA_LO12[ 	]+\.L1
[ 	]+24:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+28:[ 	]+14000004[ 	]+lu12i\.w[ 	]+\$a0,[ 	]+0
[ 	]+28:[ 	]+R_LARCH_MARK_LA[ 	]+\*ABS\*
[ 	]+28:[ 	]+R_LARCH_ABS_HI20[ 	]+\.L1
[ 	]+2c:[ 	]+03800084[ 	]+ori[ 	]+\$a0,[ 	]+\$a0,[ 	]+0x0
[ 	]+2c:[ 	]+R_LARCH_ABS_LO12[ 	]+\.L1
[ 	]+30:[ 	]+16000004[ 	]+lu32i\.d[ 	]+\$a0,[ 	]+0
[ 	]+30:[ 	]+R_LARCH_ABS64_LO20[ 	]+\.L1
[ 	]+34:[ 	]+03000084[ 	]+lu52i\.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+34:[ 	]+R_LARCH_ABS64_HI12[ 	]+\.L1
[ 	]+38:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+38:[ 	]+R_LARCH_PCALA_HI20[ 	]+\.L1
[ 	]+38:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+3c:[ 	]+02c00084[ 	]+addi\.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+3c:[ 	]+R_LARCH_PCALA_LO12[ 	]+\.L1
[ 	]+3c:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+40:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+40:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+\.L1
[ 	]+40:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+44:[ 	]+28c00084[ 	]+ld\.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+44:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+\.L1
[ 	]+44:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+48:[ 	]+14000004[ 	]+lu12i\.w[ 	]+\$a0,[ 	]+0
[ 	]+48:[ 	]+R_LARCH_TLS_LE_HI20[ 	]+TLS1
[ 	]+4c:[ 	]+03800084[ 	]+ori[ 	]+\$a0,[ 	]+\$a0,[ 	]+0x0
[ 	]+4c:[ 	]+R_LARCH_TLS_LE_LO12[ 	]+TLS1
[ 	]+50:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+50:[ 	]+R_LARCH_TLS_IE_PC_HI20[ 	]+TLS1
[ 	]+54:[ 	]+28c00084[ 	]+ld\.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+54:[ 	]+R_LARCH_TLS_IE_PC_LO12[ 	]+TLS1
[ 	]+58:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+58:[ 	]+R_LARCH_TLS_LD_PC_HI20[ 	]+TLS1
[ 	]+5c:[ 	]+02c00084[ 	]+addi\.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+5c:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+TLS1
[ 	]+5c:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+60:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+60:[ 	]+R_LARCH_TLS_GD_PC_HI20[ 	]+TLS1
[ 	]+64:[ 	]+02c00084[ 	]+addi\.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+64:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+TLS1
[ 	]+64:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
