#objdump: -dr --prefix-addresses --show-raw-insn
#name: FDPIC relocations

.*:     file format elf32-sh.*

Disassembly of section .text:
	\.\.\.
			0: R_SH_REL32	foo
			4: R_SH_FUNCDESC	foo
			8: R_SH_GOT32	foo
			c: R_SH_GOTOFF	foo
			10: R_SH_GOTFUNCDESC	foo
			14: R_SH_GOTOFFFUNCDESC	foo
