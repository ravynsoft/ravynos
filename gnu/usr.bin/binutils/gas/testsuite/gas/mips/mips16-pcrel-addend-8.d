#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative relocation with addend 8
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 4a00 	addiu	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 9a40 	lw	v0,0\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f464 4a08 	addiu	v0,9320
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f464 9a48 	lw	v0,9320\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> f222 6a14 	li	v0,4660
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f66a 4a18 	addiu	v0,22136
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> f222 6a14 	li	v0,4660
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f66a 9a58 	lw	v0,22136\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> f464 6a09 	li	v0,9321
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f4f5 4a00 	addiu	v0,-21280
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> f464 6a09 	li	v0,9321
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f4f5 9a40 	lw	v0,-21280\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
