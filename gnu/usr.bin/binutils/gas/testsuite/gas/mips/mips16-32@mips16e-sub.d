#objdump: -dr --prefix-address --show-raw-insn
#as: -32 -I$srcdir/$subdir
#name: MIPS16e ISA subset disassembly
#source: mips16e-sub.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> eac0      	.short	0xeac0
[0-9a-f]+ <[^>]*> eac0      	.short	0xeac0
[0-9a-f]+ <[^>]*> e8a0      	.short	0xe8a0
[0-9a-f]+ <[^>]*> ea80      	.short	0xea80
[0-9a-f]+ <[^>]*> eac0      	.short	0xeac0
[0-9a-f]+ <[^>]*> eac0      	.short	0xeac0
[0-9a-f]+ <[^>]*> eac0      	.short	0xeac0
[0-9a-f]+ <[^>]*> eac0      	.short	0xeac0
[0-9a-f]+ <[^>]*> e8a0      	.short	0xe8a0
[0-9a-f]+ <[^>]*> ea80      	.short	0xea80
[0-9a-f]+ <[^>]*> e8a0      	.short	0xe8a0
[0-9a-f]+ <[^>]*> ea80      	.short	0xea80
[0-9a-f]+ <[^>]*> eac0      	.short	0xeac0
[0-9a-f]+ <[^>]*> 1800 0000 	jal	00000000 <stuff>
[ 	]*[0-9a-f]+: R_MIPS16_26	foo
[0-9a-f]+ <[^>]*> 4281      	addiu	a0,v0,1
[0-9a-f]+ <[^>]*> eac0      	.short	0xeac0
[0-9a-f]+ <[^>]*> 1800 0000 	jal	00000000 <stuff>
[ 	]*[0-9a-f]+: R_MIPS16_26	foo
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6782      	move	a0,v0
[0-9a-f]+ <[^>]*> eac0      	.short	0xeac0
[0-9a-f]+ <[^>]*> 6782      	move	a0,v0
[0-9a-f]+ <[^>]*> ea80      	.short	0xea80
[0-9a-f]+ <[^>]*> 6782      	move	a0,v0
[0-9a-f]+ <[^>]*> e8a0      	.short	0xe8a0
[0-9a-f]+ <[^>]*> ec91      	.short	0xec91
[0-9a-f]+ <[^>]*> ecb1      	.short	0xecb1
[0-9a-f]+ <[^>]*> ec11      	.short	0xec11
[0-9a-f]+ <[^>]*> ec31      	.short	0xec31
[0-9a-f]+ <[^>]*> 64c1      	.short	0x64c1
[0-9a-f]+ <[^>]*> 64c0      	.short	0x64c0
[0-9a-f]+ <[^>]*> 64e2      	.short	0x64e2
[0-9a-f]+ <[^>]*> 64f2      	.short	0x64f2
[0-9a-f]+ <[^>]*> 64df      	.short	0x64df
[0-9a-f]+ <[^>]*> f010      	extend	0x10
[0-9a-f]+ <[^>]*> 64e1      	.short	0x64e1
[0-9a-f]+ <[^>]*> f004      	extend	0x4
[0-9a-f]+ <[^>]*> 64f2      	.short	0x64f2
[0-9a-f]+ <[^>]*> f308      	extend	0x308
[0-9a-f]+ <[^>]*> 64e2      	.short	0x64e2
[0-9a-f]+ <[^>]*> f30c      	extend	0x30c
[0-9a-f]+ <[^>]*> 64f2      	.short	0x64f2
[0-9a-f]+ <[^>]*> f70e      	extend	0x70e
[0-9a-f]+ <[^>]*> 64d2      	.short	0x64d2
[0-9a-f]+ <[^>]*> f30a      	extend	0x30a
[0-9a-f]+ <[^>]*> 64e2      	.short	0x64e2
[0-9a-f]+ <[^>]*> 6441      	.short	0x6441
