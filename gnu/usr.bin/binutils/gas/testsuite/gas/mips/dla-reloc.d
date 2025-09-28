#as: -64
#objdump: -dr --prefix-addresses
#name: DLA with relocation operators

.*file format.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> daddiu	a0,zero,0
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_LO16	foo
[ 	]*[0-9a-f]+: R_MIPS_NONE	.*
[ 	]*[0-9a-f]+: R_MIPS_NONE	.*
[0-9a-f]+ <[^>]*> daddiu	a0,zero,0
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_HI16	foo
[ 	]*[0-9a-f]+: R_MIPS_NONE	.*
[ 	]*[0-9a-f]+: R_MIPS_NONE	.*
[0-9a-f]+ <[^>]*> daddiu	a0,zero,-30875
[0-9a-f]+ <[^>]*> daddiu	a0,zero,4661
[0-9a-f]+ <[^>]*> daddiu	a0,a1,0
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_LO16	foo
[ 	]*[0-9a-f]+: R_MIPS_NONE	.*
[ 	]*[0-9a-f]+: R_MIPS_NONE	.*
[0-9a-f]+ <[^>]*> daddiu	a0,a1,0
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_HI16	foo
[ 	]*[0-9a-f]+: R_MIPS_NONE	.*
[ 	]*[0-9a-f]+: R_MIPS_NONE	.*
[0-9a-f]+ <[^>]*> daddiu	a0,a1,-30875
[0-9a-f]+ <[^>]*> daddiu	a0,a1,4661
[0-9a-f]+ <[^>]*> daddiu	a0,a1,0
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_LO16	foo\+0x12348765
[ 	]*[0-9a-f]+: R_MIPS_NONE	.*
[ 	]*[0-9a-f]+: R_MIPS_NONE	.*
[0-9a-f]+ <[^>]*> daddiu	a0,a1,0
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_HI16	foo\+0x12348765
[ 	]*[0-9a-f]+: R_MIPS_NONE	.*
[ 	]*[0-9a-f]+: R_MIPS_NONE	.*
[0-9a-f]+ <[^>]*> daddiu	a0,a1,0
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_GPREL16	bar
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_SUB	\*ABS\*
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_HI16	\*ABS\*
[0-9a-f]+ <[^>]*> daddiu	a0,a1,0
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_GPREL16	bar
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_SUB	\*ABS\*
[ 	]*[0-9a-f]+: R_(MICRO|)MIPS_LO16	\*ABS\*
#pass
