#name: Z8002 forward dbjnz just in range
#source: dbjnz-opcode.s -z8002
#source: branch-target2.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#objdump: -dr

.*:     file format coff-z8k


Disassembly of section \.text:

00001000 <\.text>:
    1000:	f000           	dbjnz	rh0,0x1002

00001002 <target2>:
    1002:	bd24           	ldk	r2,#0x4
