#as: -32
#objdump: -dr --prefix-addresses
#name: LA with relocation operators

.*file format.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> li	a0,0
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_LO16	foo
[0-9a-f]+ <[^>]*> li	a0,0
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_HI16	foo
[0-9a-f]+ <[^>]*> li	a0,-30875
[0-9a-f]+ <[^>]*> li	a0,4661
[0-9a-f]+ <[^>]*> addiu	a0,a1,0
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_LO16	foo
[0-9a-f]+ <[^>]*> addiu	a0,a1,0
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_HI16	foo
[0-9a-f]+ <[^>]*> addiu	a0,a1,-30875
[0-9a-f]+ <[^>]*> addiu	a0,a1,4661
[0-9a-f]+ <[^>]*> addiu	a0,a1,-30875
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_LO16	foo
[0-9a-f]+ <[^>]*> addiu	a0,a1,4661
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_HI16	foo
#pass
