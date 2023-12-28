#name: Z8002 forward relative byte load just in range
#source: ldrb-opcode.s -z8002
#source: 0filler.s -z8002 --defsym BYTES=32766
#source: branch-target.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#objdump: -dr

.*:     file format coff-z8k


Disassembly of section \.text:

00001000 <\.text>:
    1000:	3000 7fff      	ldrb	rh0,0x9003

00001004 <\.text>:
	\.\.\.

00009002 <target>:
    9002:	bd04           	ldk	r0,#0x4
