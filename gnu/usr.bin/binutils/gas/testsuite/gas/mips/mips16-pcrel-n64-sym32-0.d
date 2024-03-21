#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative operations 0 (n64, sym32)
#as: -64 -msym32
#source: mips16-pcrel-0.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0a00      	la	v0,0000000000010000 <foo>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> b200      	lw	v0,0000000000010004 <foo\+0x4>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 0aff      	la	v0,0000000000010404 <baz\+0x304>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> b2ff      	lw	v0,0000000000010408 <baz\+0x308>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f400 0a00 	la	v0,0000000000010410 <baz\+0x310>
[0-9a-f]+ <[^>]*> f400 b200 	lw	v0,0000000000010414 <baz\+0x314>
[0-9a-f]+ <[^>]*> f7ff 0a1c 	la	v0,0000000000010014 <foo\+0x14>
[0-9a-f]+ <[^>]*> f7ff b21c 	lw	v0,0000000000010018 <foo\+0x18>
[0-9a-f]+ <[^>]*> f7ef 0a1f 	la	v0,000000000001801f <baz\+0x7f1f>
[0-9a-f]+ <[^>]*> f7ef b21f 	lw	v0,0000000000018023 <baz\+0x7f23>
[0-9a-f]+ <[^>]*> f010 0a00 	la	v0,0000000000008028 <bar\+0x8028>
[0-9a-f]+ <[^>]*> f010 b200 	lw	v0,000000000000802c <bar\+0x802c>
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	L0.*\+0x7fff
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7fff
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7fff
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 4a00 	addiu	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	L0.*\+0x7fff
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7fff
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7fff
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	L0.*\+0x7fff
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7fff
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7fff
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 9a40 	lw	v0,0\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	L0.*\+0x7fff
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7fff
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7fff
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	L0.*-0x8002
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8002
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8002
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 4a00 	addiu	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	L0.*-0x8002
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8002
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8002
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	L0.*-0x8002
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8002
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8002
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f000 9a40 	lw	v0,0\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	L0.*-0x8002
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8002
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8002
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
	\.\.\.
