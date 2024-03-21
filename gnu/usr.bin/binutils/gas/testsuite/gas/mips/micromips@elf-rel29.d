#objdump: -dr --show-raw-insn
#as: -64
#name: MIPS ELF reloc 29
#source: elf-rel29.s

.*file format.*

Disassembly .*

0+0 <foo>:
.*:	41a4 1234 	lui	a0,0x1234
.*:	5884 8000 	dsll	a0,a0,0x10
.*:	5c84 5679 	daddiu	a0,a0,22137
.*:	5884 8000 	dsll	a0,a0,0x10
.*:	5c84 9abd 	daddiu	a0,a0,-25923
.*:	5884 8000 	dsll	a0,a0,0x10
.*:	fc84 def0 	lw	a0,-8464\(a0\)
#pass
