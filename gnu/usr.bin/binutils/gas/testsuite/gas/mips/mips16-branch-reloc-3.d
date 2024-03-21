#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch relocation 3
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f7ff 101e 	b	00001000 <foo>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f7ff 601e 	bteqz	00001004 <foo\+0x4>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f7ff 611e 	btnez	00001008 <foo\+0x8>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f7ff 221e 	beqz	v0,0000100c <foo\+0xc>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f7ff 2a1e 	bnez	v0,00001010 <foo\+0x10>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
