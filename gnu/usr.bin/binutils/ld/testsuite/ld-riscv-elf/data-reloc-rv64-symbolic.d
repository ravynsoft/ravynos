#source: data-reloc.s
#as: -march=rv64i -mabi=lp64 -defsym __64_bit__=1 -defsym __abs__=1 -defsym __addr__=1 -defsym __undef__=1
#ld: -m[riscv_choose_lp64_emul] -Ttext 0x8000 --defsym _start=0x0 --defsym abs=0x100 --defsym abs_local=0x200 -shared -Bsymbolic
#objdump: -dR

.*:[ 	]+file format .*


Disassembly of section .text:

0+8000 <addr_globl>:
	...
			8000: R_RISCV_RELATIVE	\*ABS\*\+0x8000

0+8008 <addr_local>:
	...
			8008: R_RISCV_RELATIVE	\*ABS\*\+0x8008
			8010: R_RISCV_RELATIVE	\*ABS\*\+0x100
    8018:	00000200          	.word	0x00000200
	...
			8020: R_RISCV_64	undef
