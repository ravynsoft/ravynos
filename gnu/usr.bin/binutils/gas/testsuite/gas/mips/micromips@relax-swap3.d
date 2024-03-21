#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS relaxed macro with branch swapping
#as: -32
#source: relax-swap3.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 41a2 0000 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MICROMIPS_HI16	bar
[0-9a-f]+ <[^>]*> 3042 0000 	addiu	v0,v0,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	bar
[0-9a-f]+ <[^>]*> 45a3      	jrc	v1
[0-9a-f]+ <[^>]*> 41a2 0000 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MICROMIPS_HI16	bar
[0-9a-f]+ <[^>]*> 3042 0000 	addiu	v0,v0,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	bar
[0-9a-f]+ <[^>]*> 40e3 fffe 	beqzc	v1,[0-9a-f]+ <[^>]*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
	\.\.\.
