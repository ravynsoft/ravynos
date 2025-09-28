#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch to absolute expression with addend
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 1000048c 	b	00002234 <foo\+0x1234>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 0411048c 	bal	0000223c <foo\+0x123c>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 0410048c 	bltzal	zero,00002244 <foo\+0x1244>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 1040048c 	beqz	v0,0000224c <foo\+0x124c>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 1440048c 	bnez	v0,00002254 <foo\+0x1254>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
