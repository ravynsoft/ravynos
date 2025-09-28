#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative operations 1 (n64, sym32)
#as: -64 -msym32
#source: mips16-pcrel-1.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> fe40      	dla	v0,0000000000010000 <foo>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> fc40      	ld	v0,0000000000010000 <foo>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> fe5f      	dla	v0,0000000000010084 <baz\+0x4>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> fc5f      	ld	v0,0000000000010100 <baz\+0x80>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f080 fe40 	dla	v0,0000000000010090 <baz\+0x10>
[0-9a-f]+ <[^>]*> f100 fc40 	ld	v0,0000000000010110 <baz\+0x90>
[0-9a-f]+ <[^>]*> f7ff fe5c 	dla	v0,0000000000010014 <foo\+0x14>
[0-9a-f]+ <[^>]*> f7ff fc5c 	ld	v0,0000000000010014 <foo\+0x14>
[0-9a-f]+ <[^>]*> f7ef fe5f 	dla	v0,000000000001801f <baz\+0x7f9f>
[0-9a-f]+ <[^>]*> f7ef fc5f 	ld	v0,000000000001801f <baz\+0x7f9f>
[0-9a-f]+ <[^>]*> f010 fe40 	dla	v0,0000000000008028 <bar\+0x8028>
[0-9a-f]+ <[^>]*> f010 fc40 	ld	v0,0000000000008028 <bar\+0x8028>
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	L0.*\+0x7fff
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7fff
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7fff
[0-9a-f]+ <[^>]*> f000 fd40 	daddiu	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	L0.*\+0x7fff
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7fff
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7fff
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	L0.*\+0x7ffb
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7ffb
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7ffb
[0-9a-f]+ <[^>]*> f000 3a40 	ld	v0,0\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	L0.*\+0x7ffb
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7ffb
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x7ffb
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	L0.*-0x8002
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8002
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8002
[0-9a-f]+ <[^>]*> f000 fd40 	daddiu	v0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	L0.*-0x8002
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8002
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8002
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	L0.*-0x8006
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8006
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8006
[0-9a-f]+ <[^>]*> f000 3a40 	ld	v0,0\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	L0.*-0x8006
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8006
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x8006
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
	\.\.\.
