#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16e2 LUI
#as: -32 -mips16 -mips32r2 -mmips16e2

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f000 6b20 	lui	v1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	.text
[0-9a-f]+ <[^>]*> f770 6c25 	lui	a0,0x8765
[0-9a-f]+ <[^>]*> f222 6d34 	lui	a1,0x1234
[0-9a-f]+ <[^>]*> f000 6e20 	lui	a2,0x0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> f020 6f28 	lui	a3,0x28
[ 	]*[0-9a-f]+: R_MIPS16_LO16	.text
[0-9a-f]+ <[^>]*> f328 6821 	lui	s0,0x4321
[0-9a-f]+ <[^>]*> f66a 6938 	lui	s1,0x5678
[0-9a-f]+ <[^>]*> f222 6a34 	lui	v0,0x1234
[0-9a-f]+ <[^>]*> f000 6b21 	lui	v1,0x1
	\.\.\.
