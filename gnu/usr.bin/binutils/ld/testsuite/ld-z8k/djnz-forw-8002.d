#name: Z8002 forward djnz just in range
#source: djnz-opcode.s -z8002
#source: branch-target.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#objdump: -dr

.*:     file format coff-z8k


Disassembly of section \.text:

00001000 <\.text>:
    1000:	f080           	djnz	r0,0x1002

00001002 <target>:
    1002:	bd04           	ldk	r0,#0x4
