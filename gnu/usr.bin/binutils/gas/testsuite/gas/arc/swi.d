#as: -mcpu=archs
#objdump: -dr --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:

00000000 <main>:
   0:	2100 0080           	add	r0,r1,r2
   4:	226f 003f           	swi
   8:	2402 0143           	sub	r3,r4,r5
