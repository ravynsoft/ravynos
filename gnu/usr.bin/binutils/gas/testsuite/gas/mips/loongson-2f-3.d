#as: -mfix-loongson2f-jump
#objdump: -M reg-names=numeric -dr
#name: ST Microelectronics Loongson-2F workarounds of Jump Instruction issue

.*:     file format .*

Disassembly of section .text:

0+000000 <.text>:
   0:	3c01cfff 	lui	\$1,0xcfff
   4:	3421ffff 	ori	\$1,\$1,0xffff
   8:	03c1f024 	and	\$30,\$30,\$1
   c:	03c00008 	jr	\$30
  10:	00000000 	nop

  14:	3c01cfff 	lui	\$1,0xcfff
  18:	3421ffff 	ori	\$1,\$1,0xffff
  1c:	03e1f824 	and	\$31,\$31,\$1
  20:	03e00008 	jr	\$31
  24:	00000000 	nop

  28:	3c01cfff 	lui	\$1,0xcfff
  2c:	3421ffff 	ori	\$1,\$1,0xffff
  30:	03c1f024 	and	\$30,\$30,\$1
  34:	03c0f809 	jalr	\$30
  38:	00000000 	nop

  3c:	00200008 	jr	\$1
  40:	00000000 	nop

  44:	08000000 	j	0x0
			44: R_MIPS_26	external_label
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
  48:	00000000 	nop
  4c:	00000000 	nop
