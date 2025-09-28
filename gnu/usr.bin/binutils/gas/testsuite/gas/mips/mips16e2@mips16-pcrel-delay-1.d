#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative operation in delay slot 1
#as: -32
#warning_output: mips16-pcrel-delay-1.l
#source: mips16-pcrel-delay-1.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 1800 0000 	jal	00000000 <bar>
[ 	]*[0-9a-f]+: R_MIPS16_26	bat
[0-9a-f]+ <[^>]*> 0aff      	la	v0,000103fc <baz\+0x2fc>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 1c00 0000 	jalx	00000000 <bar>
[ 	]*[0-9a-f]+: R_MIPS16_26	bax
[0-9a-f]+ <[^>]*> b2ff      	lw	v0,00010404 <baz\+0x304>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 1800 0000 	jal	00000000 <bar>
[ 	]*[0-9a-f]+: R_MIPS16_26	bat
[0-9a-f]+ <[^>]*> f7ef 0a1f 	la	v0,00018013 <baz\+0x7f13>
[0-9a-f]+ <[^>]*> 1c00 0000 	jalx	00000000 <bar>
[ 	]*[0-9a-f]+: R_MIPS16_26	bax
[0-9a-f]+ <[^>]*> f7ef b21f 	lw	v0,0001801b <baz\+0x7f1b>
[0-9a-f]+ <[^>]*> 1800 0000 	jal	00000000 <bar>
[ 	]*[0-9a-f]+: R_MIPS16_26	bat
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	L0.*
[0-9a-f]+ <[^>]*> f7ef 4a1b 	addiu	v0,32763
[ 	]*[0-9a-f]+: R_MIPS16_LO16	L0.*
[0-9a-f]+ <[^>]*> 1c00 0000 	jalx	00000000 <bar>
[ 	]*[0-9a-f]+: R_MIPS16_26	bax
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	L0.*
[0-9a-f]+ <[^>]*> f7ef 9a5b 	lw	v0,32763\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	L0.*
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
	\.\.\.
