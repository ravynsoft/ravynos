#as: -call_nonpic -32
#objdump: -D --section=.text --prefix-addresses
#name: .sdata and abicalls

.*:.*


Disassembly of section .text:
0+0000 <[^>]*> lui	v0,0x0
0+0004 <[^>]*> lw	v0,0\(v0\)
	\.\.\.
