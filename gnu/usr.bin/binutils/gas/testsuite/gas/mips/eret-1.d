#objdump: -d -mmips:isa32r2
#name: MIPS eret-1
#as: -32 -mfix-24k -march=24kc --no-warn

.*\.o: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	240c0000 	li	t4,0
[ 0-9a-f]+:	42000018 	eret
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	10000003 	b	[0-9a-f]+ <foo\+0x1c>
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	240a0003 	li	t2,3
[ 0-9a-f]+:	42000018 	eret
[ 0-9a-f]+:	24040000 	li	a0,0
[ 0-9a-f]+:	4200001f 	deret
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	116afffa 	beq	t3,t2,[0-9a-f]+ <foo\+0x14>
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	4200001f 	deret
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	42000018 	eret
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	42000018 	eret
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	1000fff4 	b	[0-9a-f]+ <foo\+0x1c>
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	240c0004 	li	t4,4
[ 0-9a-f]+:	4200001f 	deret
[ 0-9a-f]+:	240c0003 	li	t4,3
[ 0-9a-f]+:	42000018 	eret
[ 0-9a-f]+:	10000005 	b	[0-9a-f]+ <foo\+0x78>
[ 0-9a-f]+:	240c0003 	li	t4,3
[ 0-9a-f]+:	42000018 	eret
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	10000001 	b	[0-9a-f]+ <foo\+0x78>
[ 0-9a-f]+:	240c0003 	li	t4,3
[ 0-9a-f]+:	240c0003 	li	t4,3
[ 0-9a-f]+:	42000018 	eret
	\.\.\.
