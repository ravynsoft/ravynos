#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS branch to absolute expression with addend
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 40e0 0918 	bc	00002234 <foo\+0x1234>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
[0-9a-f]+ <[^>]*> 4060 0918 	bal	00002238 <foo\+0x1238>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4020 0918 	bltzal	zero,00002240 <foo\+0x1240>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 40e2 0918 	beqzc	v0,00002248 <foo\+0x1248>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
[0-9a-f]+ <[^>]*> 40a2 0918 	bnezc	v0,0000224c <foo\+0x124c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
