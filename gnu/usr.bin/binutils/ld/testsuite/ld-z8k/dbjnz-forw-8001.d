#name: Z8001 forward dbjnz just in range
#source: dbjnz-opcode.s -z8001
#source: branch-target2.s -z8001
#ld: -T reloc.ld -mz8001 -e 0
#objdump: -dr

.*:     file format coff-z8k


Disassembly of section \.text:

00001000 <\.text>:
    1000:	f000           	dbjnz	rh0,0x1002

00001002 <target2>:
    1002:	bd24           	ldk	r2,#0x4
