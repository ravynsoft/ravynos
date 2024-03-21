#objdump: -dr --show-raw-insn
#name: microMIPS ISA mode for .insn label references
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
00000000 <test1>:
   0:	3c030000 	lui	v1,0x0
			0: R_MIPS_HI16	\.text
   4:	2463000b 	addiu	v1,v1,11
			4: R_MIPS_LO16	\.text
00000008 <test2>:
   8:	0c00      	nop
   a:	0c00      	nop
	\.\.\.
