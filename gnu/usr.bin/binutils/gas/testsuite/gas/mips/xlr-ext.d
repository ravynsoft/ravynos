#objdump: -dr --prefix-addresses --show-raw-insn -mmips:xlr
#name: XLRs native MIPS64 extensions
#as: -march=xlr

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 3c000000 	lui	zero,0x0
0+0004 <[^>]*> 8c010001 	lw	at,1\(zero\)
0+0008 <[^>]*> 8c020002 	lw	v0,2\(zero\)
0+000c <[^>]*> 70221838 	daddwc	v1,at,v0
0+0010 <[^>]*> 70230010 	ldaddw	v1,at
0+0014 <[^>]*> 70230011 	ldaddwu	v1,at
0+0018 <[^>]*> 70230012 	ldaddd	v1,at
0+001c <[^>]*> 70230014 	swapw	v1,at
0+0020 <[^>]*> 70230015 	swapwu	v1,at
0+0024 <[^>]*> 4a000003 	c2	0x3
0+0028 <[^>]*> 4a000002 	c2	0x2
0+002c <[^>]*> 4a000001 	c2	0x1
	\.\.\.
