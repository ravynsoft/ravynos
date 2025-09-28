#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS PIC option
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 8f820000 	lw	v0,0\(gp\)
[ 	]*[0-9a-f]+: R_MIPS_GOT16	bar
[0-9a-f]+ <[^>]*> 3c020000 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS_HI16	bar
[0-9a-f]+ <[^>]*> 24420000 	addiu	v0,v0,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	bar
[0-9a-f]+ <[^>]*> 8f820000 	lw	v0,0\(gp\)
[ 	]*[0-9a-f]+: R_MIPS_GOT16	bar
[0-9a-f]+ <[^>]*> 03e00008 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
