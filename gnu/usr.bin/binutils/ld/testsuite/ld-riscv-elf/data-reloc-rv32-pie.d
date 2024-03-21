#source: data-reloc.s
#as: -march=rv32i -mabi=ilp32 -defsym __abs__=1 -defsym __addr__=1
#ld: -m[riscv_choose_ilp32_emul] -Ttext 0x8000 --defsym _start=0x0 --defsym abs=0x100 --defsym abs_local=0x200 -pie
#objdump: -dR

.*:[ 	]+file format .*

Disassembly of section .text:

0+8000 <addr_globl>:
    8000:	00000000          	.word	0x00000000
			8000: R_RISCV_RELATIVE	\*ABS\*\+0x8000

0+8004 <addr_local>:
    8004:	00000000          	.word	0x00000000
			8004: R_RISCV_RELATIVE	\*ABS\*\+0x8004
    8008:	00000100          	.word	0x00000100
    800c:	00000200          	.word	0x00000200
