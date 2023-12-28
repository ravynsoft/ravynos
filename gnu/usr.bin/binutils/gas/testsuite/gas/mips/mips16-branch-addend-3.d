#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch relocation with addend 3
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f101 1018 	b	00002234 <foo\+0x1234>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f101 6018 	bteqz	00002238 <foo\+0x1238>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f101 6118 	btnez	0000223c <foo\+0x123c>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f101 2218 	beqz	v0,00002240 <foo\+0x1240>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f101 2a18 	bnez	v0,00002244 <foo\+0x1244>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
