#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative operation in delay slot 0
#as: -32
#warning_output: mips16-pcrel-delay-0.l

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> ec00      	jr	a0
[0-9a-f]+ <[^>]*> 0aff      	la	v0,000103fc <baz\+0x2fc>
[0-9a-f]+ <[^>]*> e820      	jr	ra
[0-9a-f]+ <[^>]*> b2ff      	lw	v0,00010400 <baz\+0x300>
[0-9a-f]+ <[^>]*> ec00      	jr	a0
[0-9a-f]+ <[^>]*> f7ef 0a1f 	la	v0,00018007 <baz\+0x7f07>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> e820      	jr	ra
[0-9a-f]+ <[^>]*> f7ef b21f 	lw	v0,0001800f <baz\+0x7f0f>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> ec00      	jr	a0
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	L0.*
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f7ef 4a1d 	addiu	v0,32765
[ 	]*[0-9a-f]+: R_MIPS16_LO16	L0.*
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> e820      	jr	ra
[0-9a-f]+ <[^>]*> f000 6a00 	li	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	L0.*
[0-9a-f]+ <[^>]*> f400 3240 	sll	v0,16
[0-9a-f]+ <[^>]*> f7ef 9a5d 	lw	v0,32765\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	L0.*
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
	\.\.\.
