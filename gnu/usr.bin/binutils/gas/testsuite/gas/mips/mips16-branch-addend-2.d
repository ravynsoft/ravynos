#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch relocation with addend 2
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f101 1018 	b	00002234 <bar\+0x1204>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f101 6018 	bteqz	00002238 <bar\+0x1208>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f101 6118 	btnez	0000223c <bar\+0x120c>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f101 2218 	beqz	v0,00002240 <bar\+0x1210>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f101 2a18 	bnez	v0,00002244 <bar\+0x1214>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
