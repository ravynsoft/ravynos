#source: bug-1403.s
#as: -m68hc11
#ld: -m m68hc11elf --relax
#objdump: -d --prefix-addresses -r

.*: +file format elf32-m68hc11

Disassembly of section .text:
0+8000 <_start> bset	\*0x0+ <__bss_size>, \#0x04
0+8003 <L1> bra	0x0+8005 <toto>
0+8005 <toto> rts
