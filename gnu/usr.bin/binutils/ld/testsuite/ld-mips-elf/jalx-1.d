#name: MIPS jalx-1
#source: jalx-1.s
#ld: -T jalx-1.ld
#objdump: -d

.*: +file format .*mips.*

Disassembly of section \.text:

0*88000000 <test>:
 *88000000:	f200 0002 	jalx	88000008 <test1>
 *88000004:	0000 0000 	nop

0*88000008 <test1>:
 *88000008:	00851821 	addu	v1,a0,a1
	\.\.\.
