#as: -mla-global-with-abs
#objdump: -dr
#skip: loongarch32-*-*

.*:[    ]+file format .*


Disassembly of section .text:

00000000.* <.L1>:
[ 	]+0:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+0:[ 	]+R_LARCH_PCALA_HI20[ 	]+.L1
[ 	]+0:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+4:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+4:[ 	]+R_LARCH_PCALA_LO12[ 	]+.L1
[ 	]+4:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+8:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+8:[ 	]+R_LARCH_PCALA64_LO20[ 	]+.L1
[ 	]+c:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+c:[ 	]+R_LARCH_PCALA64_HI12[ 	]+.L1
[ 	]+10:[ 	]+00109484[ 	]+add.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+14:[ 	]+14000004[ 	]+lu12i.w[ 	]+\$a0,[ 	]+0
[ 	]+14:[ 	]+R_LARCH_MARK_LA[ 	]+\*ABS\*
[ 	]+14:[ 	]+R_LARCH_ABS_HI20[ 	]+.L1
[ 	]+18:[ 	]+03800084[ 	]+ori[ 	]+\$a0,[ 	]+\$a0,[ 	]+0x0
[ 	]+18:[ 	]+R_LARCH_ABS_LO12[ 	]+.L1
[ 	]+1c:[ 	]+16000004[ 	]+lu32i.d[ 	]+\$a0,[ 	]+0
[ 	]+1c:[ 	]+R_LARCH_ABS64_LO20[ 	]+.L1
[ 	]+20:[ 	]+03000084[ 	]+lu52i.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+20:[ 	]+R_LARCH_ABS64_HI12[ 	]+.L1
[ 	]+24:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+24:[ 	]+R_LARCH_PCALA_HI20[ 	]+.L1
[ 	]+24:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+28:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+28:[ 	]+R_LARCH_PCALA_LO12[ 	]+.L1
[ 	]+28:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+2c:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+2c:[ 	]+R_LARCH_PCALA64_LO20[ 	]+.L1
[ 	]+30:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+30:[ 	]+R_LARCH_PCALA64_HI12[ 	]+.L1
[ 	]+34:[ 	]+00109484[ 	]+add.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+38:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+38:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+.L1
[ 	]+38:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+3c:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+3c:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+.L1
[ 	]+3c:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+40:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+40:[ 	]+R_LARCH_GOT64_PC_LO20[ 	]+.L1
[ 	]+44:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+44:[ 	]+R_LARCH_GOT64_PC_HI12[ 	]+.L1
[ 	]+48:[ 	]+380c1484[ 	]+ldx.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+4c:[ 	]+14000004[ 	]+lu12i.w[ 	]+\$a0,[ 	]+0
[ 	]+4c:[ 	]+R_LARCH_TLS_LE_HI20[ 	]+TLS1
[ 	]+50:[ 	]+03800084[ 	]+ori[ 	]+\$a0,[ 	]+\$a0,[ 	]+0x0
[ 	]+50:[ 	]+R_LARCH_TLS_LE_LO12[ 	]+TLS1
[ 	]+54:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+54:[ 	]+R_LARCH_TLS_IE_PC_HI20[ 	]+TLS1
[ 	]+58:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+58:[ 	]+R_LARCH_TLS_IE_PC_LO12[ 	]+TLS1
[ 	]+5c:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+5c:[ 	]+R_LARCH_TLS_IE64_PC_LO20[ 	]+TLS1
[ 	]+60:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+60:[ 	]+R_LARCH_TLS_IE64_PC_HI12[ 	]+TLS1
[ 	]+64:[ 	]+380c1484[ 	]+ldx.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+68:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+68:[ 	]+R_LARCH_TLS_LD_PC_HI20[ 	]+TLS1
[ 	]+6c:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+6c:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+TLS1
[ 	]+6c:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+70:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+70:[ 	]+R_LARCH_GOT64_PC_LO20[ 	]+TLS1
[ 	]+74:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+74:[ 	]+R_LARCH_GOT64_PC_HI12[ 	]+TLS1
[ 	]+78:[ 	]+00109484[ 	]+add.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+7c:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+7c:[ 	]+R_LARCH_TLS_GD_PC_HI20[ 	]+TLS1
[ 	]+80:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+80:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+TLS1
[ 	]+80:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+84:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+84:[ 	]+R_LARCH_GOT64_PC_LO20[ 	]+TLS1
[ 	]+88:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+88:[ 	]+R_LARCH_GOT64_PC_HI12[ 	]+TLS1
[ 	]+8c:[ 	]+00109484[ 	]+add.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
