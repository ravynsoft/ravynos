#objdump: -d -mmips:isa32r2
#name: MIPS eret-2
#as: -32 -mfix-24k -march=24kc --no-warn

.*\.o: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	42000018 	eret
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	1000fffd 	b	[0-9a-f]+ <foo>
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	42000018 	eret
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	1000fffd 	b	[0-9a-f]+ <foo\+0x10>
[ 0-9a-f]+:	00000006 	srlv	zero,zero,zero
	\.\.\.
