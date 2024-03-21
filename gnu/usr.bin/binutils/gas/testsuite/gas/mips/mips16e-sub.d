#objdump: -dr --prefix-address --show-raw-insn
#as: -32 -I$srcdir/$subdir
#name: MIPS16e ISA subset disassembly

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> eac0      	jalrc	v0
[0-9a-f]+ <[^>]*> eac0      	jalrc	v0
[0-9a-f]+ <[^>]*> e8a0      	jrc	ra
[0-9a-f]+ <[^>]*> ea80      	jrc	v0
[0-9a-f]+ <[^>]*> eac0      	jalrc	v0
[0-9a-f]+ <[^>]*> eac0      	jalrc	v0
[0-9a-f]+ <[^>]*> eac0      	jalrc	v0
[0-9a-f]+ <[^>]*> eac0      	jalrc	v0
[0-9a-f]+ <[^>]*> e8a0      	jrc	ra
[0-9a-f]+ <[^>]*> ea80      	jrc	v0
[0-9a-f]+ <[^>]*> e8a0      	jrc	ra
[0-9a-f]+ <[^>]*> ea80      	jrc	v0
[0-9a-f]+ <[^>]*> eac0      	jalrc	v0
[0-9a-f]+ <[^>]*> 1800 0000 	jal	00000000 <stuff>
[ 	]*[0-9a-f]+: R_MIPS16_26	foo
[0-9a-f]+ <[^>]*> 4281      	addiu	a0,v0,1
[0-9a-f]+ <[^>]*> eac0      	jalrc	v0
[0-9a-f]+ <[^>]*> 1800 0000 	jal	00000000 <stuff>
[ 	]*[0-9a-f]+: R_MIPS16_26	foo
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6782      	move	a0,v0
[0-9a-f]+ <[^>]*> eac0      	jalrc	v0
[0-9a-f]+ <[^>]*> 6782      	move	a0,v0
[0-9a-f]+ <[^>]*> ea80      	jrc	v0
[0-9a-f]+ <[^>]*> 6782      	move	a0,v0
[0-9a-f]+ <[^>]*> e8a0      	jrc	ra
[0-9a-f]+ <[^>]*> ec91      	seb	a0
[0-9a-f]+ <[^>]*> ecb1      	seh	a0
[0-9a-f]+ <[^>]*> ec11      	zeb	a0
[0-9a-f]+ <[^>]*> ec31      	zeh	a0
[0-9a-f]+ <[^>]*> 64c1      	save	8,ra
[0-9a-f]+ <[^>]*> 64c0      	save	128,ra
[0-9a-f]+ <[^>]*> 64e2      	save	16,ra,s0
[0-9a-f]+ <[^>]*> 64f2      	save	16,ra,s0-s1
[0-9a-f]+ <[^>]*> 64df      	save	120,ra,s1
[0-9a-f]+ <[^>]*> f010 64e1 	save	136,ra,s0
[0-9a-f]+ <[^>]*> f004 64f2 	save	a0,16,ra,s0-s1
[0-9a-f]+ <[^>]*> f308 64e2 	save	a0-a1,16,ra,s0,s2-s4
[0-9a-f]+ <[^>]*> f30c 64f2 	save	a0-a2,16,ra,s0-s4
[0-9a-f]+ <[^>]*> f70e 64d2 	save	a0-a3,16,ra,s1-s8
[0-9a-f]+ <[^>]*> f30a 64e2 	save	a0-a1,16,ra,s0,s2-s4,a2-a3
[0-9a-f]+ <[^>]*> 6441      	restore	8,ra
