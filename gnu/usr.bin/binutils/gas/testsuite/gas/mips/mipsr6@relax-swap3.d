#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS relaxed macro with branch swapping
#as: -32
#source: relax-swap3.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 3c020000 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS_HI16	bar
[0-9a-f]+ <[^>]*> 24420000 	addiu	v0,v0,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	bar
[0-9a-f]+ <[^>]*> 00600009 	jr	v1
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 3c020000 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS_HI16	bar
[0-9a-f]+ <[^>]*> 24420000 	addiu	v0,v0,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	bar
[0-9a-f]+ <[^>]*> 1060ffff 	beqz	v1,[0-9a-f]+ <[^>]*>
[ 	]*[0-9a-f]+: R_MIPS_PC16	.L.*1
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
