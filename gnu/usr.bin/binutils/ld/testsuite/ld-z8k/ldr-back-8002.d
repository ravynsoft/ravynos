#name: Z8002 backward relative load just in range
#source: branch-target.s -z8002
#source: 0filler.s -z8002 --defsym BYTES=32762
#source: ldr-opcode.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#objdump: -dr

.*:     file format coff-z8k


Disassembly of section \.text:

00001000 <target>:
    1000:	bd04           	ldk	r0,#0x4

00001002 <\.text>:
	\.\.\.

00008ffc <\.text>:
    8ffc:	3100 8000      	ldr	r0,0x1000
