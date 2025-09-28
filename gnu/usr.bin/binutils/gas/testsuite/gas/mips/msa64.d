#objdump: -dr --prefix-addresses --show-raw-insn -Mmsa
#name: MSA64 instructions
#as: -64 -mmsa

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 7b03945e 	fill\.d	\$w17,s2
[0-9a-f]+ <[^>]*> 78b8c5d9 	copy_s\.d	s7,\$w24\[0\]
[0-9a-f]+ <[^>]*> 78b9d659 	copy_s\.d	t9,\$w26\[1\]
[0-9a-f]+ <[^>]*> 78f020d9 	copy_u\.w	v1,\$w4\[0\]
[0-9a-f]+ <[^>]*> 78f33159 	copy_u\.w	a1,\$w6\[3\]
[0-9a-f]+ <[^>]*> 7938c5d9 	insert\.d	\$w23\[0\],t8
[0-9a-f]+ <[^>]*> 7939d659 	insert\.d	\$w25\[1\],k0
[0-9a-f]+ <[^>]*> 035bc815 	dlsa	t9,k0,k1,0x1
[0-9a-f]+ <[^>]*> 03bee0d5 	dlsa	gp,sp,s8,0x4
	\.\.\.
