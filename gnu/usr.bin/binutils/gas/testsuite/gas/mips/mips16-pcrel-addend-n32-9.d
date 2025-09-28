#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative relocation with addend 9 (n32)
#as: -n32
#source: mips16-pcrel-addend-9.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 fd40 	daddiu	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 3a40 	ld	v0,0\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar\+0x2468
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 fd40 	daddiu	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar\+0x2468
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar\+0x2468
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 3a40 	ld	v0,0\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar\+0x2468
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar\+0x12345678
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 fd40 	daddiu	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar\+0x12345678
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar\+0x12345678
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 3a40 	ld	v0,0\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar\+0x12345678
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar\+0x2468ace0
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 fd40 	daddiu	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar\+0x2468ace0
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar\+0x2468ace0
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 3a40 	ld	v0,0\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar\+0x2468ace0
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
