#objdump: -dr --prefix-addresses --show-raw-insn
#name: SH2a PIC relocations
#as: -isa=sh2a

.*:     file format elf32-sh.*

Disassembly of section .text:
0x00000000 01 00 00 00 	movi20	#0,r1
			0: R_SH_GOT20	foo
0x00000004 01 00 00 00 	movi20	#0,r1
			4: R_SH_GOTOFF20	foo
0x00000008 01 00 00 00 	movi20	#0,r1
			8: R_SH_GOTFUNCDESC20	foo
0x0000000c 01 00 00 00 	movi20	#0,r1
			c: R_SH_GOTOFFFUNCDESC20	foo
