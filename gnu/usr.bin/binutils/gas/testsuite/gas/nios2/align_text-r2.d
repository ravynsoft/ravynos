#objdump: -dr
#name: NIOS2 R2 align_test
#as: -march=r2
#source: align_text.s

# Test alignment in text sections.

.*: +file format elf32-littlenios2

Disassembly of section .text:
00000000 <label-0x20>:
   0:	00000000 	call	0 <label-0x20>
   4:	c4000020 	nop
   8:	c4000020 	nop
   c:	c4000020 	nop
  10:	c4000020 	nop
  14:	c4000020 	nop
  18:	c4000020 	nop
  1c:	c4000020 	nop

00000020 <label>:
  20:	c4000020 	nop
00000024 <label2>:
	...
