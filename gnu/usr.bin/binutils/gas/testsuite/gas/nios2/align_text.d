#objdump: -dr
#name: NIOS2 align_test

# Test alignment in text sections.

.*: +file format elf32-littlenios2

Disassembly of section .text:
00000000 <label-0x20>:
   0:	00000000 	call	0 <label-0x20>
   4:	0001883a 	nop
   8:	0001883a 	nop
   c:	0001883a 	nop
  10:	0001883a 	nop
  14:	0001883a 	nop
  18:	0001883a 	nop
  1c:	0001883a 	nop

00000020 <label>:
  20:	0001883a 	nop
00000024 <label2>:
	...
