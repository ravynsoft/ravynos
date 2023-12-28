#source: bug-1417.s
#as: -m68hc11
#ld: -m m68hc11elf --relax
#objdump: -d --prefix-addresses -r

.*: +file format elf32-m68hc11

Disassembly of section .text:
0+8000 <_start> tst	0x0+ <__bss_size>
0+8003 <_start\+0x3> bne	0x0+8007 <L1>
0+8005 <_start\+0x5> bsr	0x0+800b <foo>
0+8007 <L1> bset	\*0x0+ <__bss_size>, \#0x04
0+800a <L2> rts
0+800b <foo> rts
