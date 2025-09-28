#name: Z8001 forward relative load just in range
#source: ldr-opcode.s -z8001
#source: 0filler.s -z8001 --defsym BYTES=32766
#source: branch-target.s -z8001
#ld: -T reloc.ld -mz8001 -e 0
#objdump: -dr

.*:     file format coff-z8k


Disassembly of section \.text:

00001000 <\.text>:
    1000:	3100 7ffe      	ldr	r0,0x9002

00001004 <\.text>:
	\.\.\.

00009002 <target>:
    9002:	bd04           	ldk	r0,#0x4
