#as:
#objdump: -dr

.*:[    ]+file format .*


Disassembly of section .text:

00000000.* <.L1>:
[ 	]+0:[ 	]+00150004[ 	]+move[ 	]+\$a0,[ 	]+\$zero
[ 	]+4:[ 	]+02bffc04[ 	]+li\.w[ 	]+\$a0,[ 	]+-1
[ 	]+8:[ 	]+00150004[ 	]+move[ 	]+\$a0,[ 	]+\$zero
[ 	]+c:[ 	]+02bffc04[ 	]+li\.w[ 	]+\$a0,[ 	]+-1
[ 	]+10:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+10:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+.L1
[ 	]+10:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+14:[ 	]+28c00084[ 	]+ld.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+14:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+.L1
[ 	]+14:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+18:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+18:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+.L1
[ 	]+18:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+1c:[ 	]+28c00084[ 	]+ld.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+1c:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+.L1
[ 	]+1c:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+20:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+20:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+.L1
[ 	]+20:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+24:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+24:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+.L1
[ 	]+24:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+28:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+28:[ 	]+R_LARCH_GOT64_PC_LO20[ 	]+.L1
[ 	]+2c:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+2c:[ 	]+R_LARCH_GOT64_PC_HI12[ 	]+.L1
[ 	]+30:[ 	]+380c1484[ 	]+ldx.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+34:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+34:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+.L1
[ 	]+34:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+38:[ 	]+28c00084[ 	]+ld.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+38:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+.L1
[ 	]+38:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+3c:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+3c:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+.L1
[ 	]+3c:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+40:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+40:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+.L1
[ 	]+40:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+44:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+44:[ 	]+R_LARCH_GOT64_PC_LO20[ 	]+.L1
[ 	]+48:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+48:[ 	]+R_LARCH_GOT64_PC_HI12[ 	]+.L1
[ 	]+4c:[ 	]+380c1484[ 	]+ldx.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+50:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+50:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+.L1
[ 	]+50:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+54:[ 	]+28c00084[ 	]+ld.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+54:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+.L1
[ 	]+54:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+58:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+58:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+.L1
[ 	]+58:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+5c:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+5c:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+.L1
[ 	]+5c:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+60:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+60:[ 	]+R_LARCH_GOT64_PC_LO20[ 	]+.L1
[ 	]+64:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+64:[ 	]+R_LARCH_GOT64_PC_HI12[ 	]+.L1
[ 	]+68:[ 	]+380c1484[ 	]+ldx.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+6c:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+6c:[ 	]+R_LARCH_PCALA_HI20[ 	]+.L1
[ 	]+6c:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+70:[ 	]+02c00084[ 	]+addi.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+70:[ 	]+R_LARCH_PCALA_LO12[ 	]+.L1
[ 	]+70:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+74:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+74:[ 	]+R_LARCH_PCALA_HI20[ 	]+.L1
[ 	]+74:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+78:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+78:[ 	]+R_LARCH_PCALA_LO12[ 	]+.L1
[ 	]+78:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+7c:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+7c:[ 	]+R_LARCH_PCALA64_LO20[ 	]+.L1
[ 	]+80:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+80:[ 	]+R_LARCH_PCALA64_HI12[ 	]+.L1
[ 	]+84:[ 	]+00109484[ 	]+add.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+88:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+88:[ 	]+R_LARCH_PCALA_HI20[ 	]+.L1
[ 	]+88:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+8c:[ 	]+02c00084[ 	]+addi.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+8c:[ 	]+R_LARCH_PCALA_LO12[ 	]+.L1
[ 	]+8c:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+90:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+90:[ 	]+R_LARCH_PCALA_HI20[ 	]+.L1
[ 	]+90:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+94:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+94:[ 	]+R_LARCH_PCALA_LO12[ 	]+.L1
[ 	]+94:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+98:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+98:[ 	]+R_LARCH_PCALA64_LO20[ 	]+.L1
[ 	]+9c:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+9c:[ 	]+R_LARCH_PCALA64_HI12[ 	]+.L1
[ 	]+a0:[ 	]+00109484[ 	]+add.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+a4:[ 	]+14000004[ 	]+lu12i.w[ 	]+\$a0,[ 	]+0
[ 	]+a4:[ 	]+R_LARCH_MARK_LA[ 	]+\*ABS\*
[ 	]+a4:[ 	]+R_LARCH_ABS_HI20[ 	]+.L1
[ 	]+a8:[ 	]+03800084[ 	]+ori[ 	]+\$a0,[ 	]+\$a0,[ 	]+0x0
[ 	]+a8:[ 	]+R_LARCH_ABS_LO12[ 	]+.L1
[ 	]+ac:[ 	]+16000004[ 	]+lu32i.d[ 	]+\$a0,[ 	]+0
[ 	]+ac:[ 	]+R_LARCH_ABS64_LO20[ 	]+.L1
[ 	]+b0:[ 	]+03000084[ 	]+lu52i.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+b0:[ 	]+R_LARCH_ABS64_HI12[ 	]+.L1
[ 	]+b4:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+b4:[ 	]+R_LARCH_PCALA_HI20[ 	]+.L1
[ 	]+b4:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+b8:[ 	]+02c00084[ 	]+addi.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+b8:[ 	]+R_LARCH_PCALA_LO12[ 	]+.L1
[ 	]+b8:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+bc:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+bc:[ 	]+R_LARCH_PCALA_HI20[ 	]+.L1
[ 	]+bc:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+c0:[ 	]+02c00084[ 	]+addi.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+c0:[ 	]+R_LARCH_PCALA_LO12[ 	]+.L1
[ 	]+c0:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+c4:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+c4:[ 	]+R_LARCH_PCALA_HI20[ 	]+.L1
[ 	]+c4:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+c8:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+c8:[ 	]+R_LARCH_PCALA_LO12[ 	]+.L1
[ 	]+c8:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+cc:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+cc:[ 	]+R_LARCH_PCALA64_LO20[ 	]+.L1
[ 	]+d0:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+d0:[ 	]+R_LARCH_PCALA64_HI12[ 	]+.L1
[ 	]+d4:[ 	]+00109484[ 	]+add.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+d8:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+d8:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+.L1
[ 	]+d8:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+dc:[ 	]+28c00084[ 	]+ld.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+dc:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+.L1
[ 	]+dc:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+e0:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+e0:[ 	]+R_LARCH_GOT_PC_HI20[ 	]+.L1
[ 	]+e0:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+e4:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+e4:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+.L1
[ 	]+e4:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+e8:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+e8:[ 	]+R_LARCH_GOT64_PC_LO20[ 	]+.L1
[ 	]+ec:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+ec:[ 	]+R_LARCH_GOT64_PC_HI12[ 	]+.L1
[ 	]+f0:[ 	]+380c1484[ 	]+ldx.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+f4:[ 	]+14000004[ 	]+lu12i.w[ 	]+\$a0,[ 	]+0
[ 	]+f4:[ 	]+R_LARCH_TLS_LE_HI20[ 	]+TLS1
[ 	]+f8:[ 	]+03800084[ 	]+ori[ 	]+\$a0,[ 	]+\$a0,[ 	]+0x0
[ 	]+f8:[ 	]+R_LARCH_TLS_LE_LO12[ 	]+TLS1
[ 	]+fc:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+fc:[ 	]+R_LARCH_TLS_IE_PC_HI20[ 	]+TLS1
[ 	]+100:[ 	]+28c00084[ 	]+ld.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+100:[ 	]+R_LARCH_TLS_IE_PC_LO12[ 	]+TLS1
[ 	]+104:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+104:[ 	]+R_LARCH_TLS_IE_PC_HI20[ 	]+TLS1
[ 	]+108:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+108:[ 	]+R_LARCH_TLS_IE_PC_LO12[ 	]+TLS1
[ 	]+10c:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+10c:[ 	]+R_LARCH_TLS_IE64_PC_LO20[ 	]+TLS1
[ 	]+110:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+110:[ 	]+R_LARCH_TLS_IE64_PC_HI12[ 	]+TLS1
[ 	]+114:[ 	]+380c1484[ 	]+ldx.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+118:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+118:[ 	]+R_LARCH_TLS_LD_PC_HI20[ 	]+TLS1
[ 	]+11c:[ 	]+02c00084[ 	]+addi.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+11c:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+TLS1
[ 	]+11c:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+120:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+120:[ 	]+R_LARCH_TLS_LD_PC_HI20[ 	]+TLS1
[ 	]+124:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+124:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+TLS1
[ 	]+124:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+128:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+128:[ 	]+R_LARCH_GOT64_PC_LO20[ 	]+TLS1
[ 	]+12c:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+12c:[ 	]+R_LARCH_GOT64_PC_HI12[ 	]+TLS1
[ 	]+130:[ 	]+00109484[ 	]+add.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
[ 	]+134:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+134:[ 	]+R_LARCH_TLS_GD_PC_HI20[ 	]+TLS1
[ 	]+138:[ 	]+02c00084[ 	]+addi.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+0
[ 	]+138:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+TLS1
[ 	]+138:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+13c:[ 	]+1a000004[ 	]+pcalau12i[ 	]+\$a0,[ 	]+0
[ 	]+13c:[ 	]+R_LARCH_TLS_GD_PC_HI20[ 	]+TLS1
[ 	]+140:[ 	]+02c00005[ 	]+li\.d[ 	]+\$a1,[ 	]+0
[ 	]+140:[ 	]+R_LARCH_GOT_PC_LO12[ 	]+TLS1
[ 	]+140:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+144:[ 	]+16000005[ 	]+lu32i.d[ 	]+\$a1,[ 	]+0
[ 	]+144:[ 	]+R_LARCH_GOT64_PC_LO20[ 	]+TLS1
[ 	]+148:[ 	]+030000a5[ 	]+lu52i.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+148:[ 	]+R_LARCH_GOT64_PC_HI12[ 	]+TLS1
[ 	]+14c:[ 	]+00109484[ 	]+add.d[ 	]+\$a0,[ 	]+\$a0,[ 	]+\$a1
