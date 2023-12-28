#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch relocation with addend 4
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f7ef 101f 	b	00011002 <foo\+0x10002>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f7ef 601f 	bteqz	00011006 <foo\+0x10006>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f7ef 611f 	btnez	0001100a <foo\+0x1000a>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f7ef 221f 	beqz	v0,0001100e <foo\+0x1000e>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f7ef 2a1f 	bnez	v0,00011012 <foo\+0x10012>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f010 1000 	b	ffff1018 <foo\+0xffff0018>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f010 6000 	bteqz	ffff101c <foo\+0xffff001c>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f010 6100 	btnez	ffff1020 <foo\+0xffff0020>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f010 2200 	beqz	v0,ffff1024 <foo\+0xffff0024>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> f010 2a00 	bnez	v0,ffff1028 <foo\+0xffff0028>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
