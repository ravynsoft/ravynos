#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS relaxed macro with branch swapping
#as: -32
#source: relax-swap3.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f000 4a00 	addiu	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> eb80      	jrc	v1
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f000 4a00 	addiu	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> 2300      	beqz	v1,[0-9a-f]+ <[^>]*>
	\.\.\.
