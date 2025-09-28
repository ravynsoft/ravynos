#as: -a32 -mbig -mvle
#objdump: -d -Mspe -Mraw
#name: Validate SPE raw instructions

.*: +file format elf.*-powerpc.*

Disassembly of section .text:

00000000 <.text>:
   0:	10 01 12 04 	evsubfw r0,r1,r2
   4:	10 01 12 04 	evsubfw r0,r1,r2
   8:	10 1f 12 06 	evsubifw r0,31,r2
   c:	10 1f 12 06 	evsubifw r0,31,r2
  10:	10 01 0a 18 	evnor   r0,r1,r1
  14:	10 01 0a 18 	evnor   r0,r1,r1
