#objdump: -d -mmips:isa32r2
#name: MIPS eret-3
#as: -32 -mfix-24k -march=24kc --no-warn

.*\.o: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <foo>:
[ 0-9a-f]+:	42000018 	eret
[ 0-9a-f]+:	00000000 	nop

[0-9a-f]+ <bar>:
[ 0-9a-f]+:	10800002 	beqz	a0,[0-9a-f]+ <bar\+0xc>
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	aca40000 	sw	a0,0\(a1\)
[ 0-9a-f]+:	03e00008 	jr	ra
[ 0-9a-f]+:	00000000 	nop
	\.\.\.
