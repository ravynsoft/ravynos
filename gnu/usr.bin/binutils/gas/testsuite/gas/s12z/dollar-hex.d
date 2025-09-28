#objdump: -d
#name:    Dollar sign as literal hex prefix
#source:  dollar-hex.s --mdollar-hex

tmpdir/dollar-hex.o:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1c bc e0 18 	mov.b d0, \(24,s\)
   4:	dc fa 12 34 	neg.b 1193046
   8:	56 
   9:	98 12 34 56 	ld x, #1193046
   d:	aa e1 dd    	jmp \(-35,s\)

