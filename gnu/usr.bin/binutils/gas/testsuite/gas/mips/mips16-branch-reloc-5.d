#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch relocation 5
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text\.bar:
	\.\.\.
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.

Disassembly of section \.text\.foo:
[0-9a-f]+ <[^>]*> f7ff 101e 	b	00000000 <foo>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f7ff 601e 	bteqz	00000004 <foo\+0x4>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f7ff 611e 	btnez	00000008 <foo\+0x8>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f7ff 221e 	beqz	v0,0000000c <foo\+0xc>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f7ff 2a1e 	bnez	v0,00000010 <foo\+0x10>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
