#objdump: -dr --show-raw-insn
#as: -64
#name: MIPS ELF reloc 29
#source: elf-rel29.s

.*file format.*

Disassembly .*

0+0 <foo>:
.*:	3c041234 	lui	a0,0x1234
.*:	00042438 	dsll	a0,a0,0x10
.*:	64845679 	daddiu	a0,a0,22137
.*:	00042438 	dsll	a0,a0,0x10
.*:	64849abd 	daddiu	a0,a0,-25923
.*:	00042438 	dsll	a0,a0,0x10
.*:	8c84def0 	lw	a0,-8464\(a0\)
#pass
