#objdump: -dr --prefix-addresses --show-raw-insn -Mmsa
#name: MSA64 instructions
#source: msa64.s
#as: -64 -mmsa

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 5b03 946e 	fill\.d	\$w17,s2
[0-9a-f]+ <[^>]*> 58b8 c5d6 	copy_s\.d	s7,\$w24\[0\]
[0-9a-f]+ <[^>]*> 58b9 d656 	copy_s\.d	t9,\$w26\[1\]
[0-9a-f]+ <[^>]*> 58f0 20d6 	copy_u\.w	v1,\$w4\[0\]
[0-9a-f]+ <[^>]*> 58f3 3156 	copy_u\.w	a1,\$w6\[3\]
[0-9a-f]+ <[^>]*> 5938 c5d6 	insert\.d	\$w23\[0\],t8
[0-9a-f]+ <[^>]*> 5939 d656 	insert\.d	\$w25\[1\],k0
[0-9a-f]+ <[^>]*> 5b7a c820 	dlsa	t9,k0,k1,0x1
[0-9a-f]+ <[^>]*> 5bdd e0e0 	dlsa	gp,sp,s8,0x4
	\.\.\.
