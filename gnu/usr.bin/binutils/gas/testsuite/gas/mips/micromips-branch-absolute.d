#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS branch to absolute expression
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 40e0 fffe 	bc	00001000 <foo>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	00001004 <foo\+0x4>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4020 fffe 	bltzal	zero,0000100c <foo\+0xc>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 40e2 fffe 	beqzc	v0,00001014 <foo\+0x14>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
[0-9a-f]+ <[^>]*> 40a2 fffe 	bnezc	v0,00001018 <foo\+0x18>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
