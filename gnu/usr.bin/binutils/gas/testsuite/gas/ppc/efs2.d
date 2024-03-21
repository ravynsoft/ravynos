#as: -a32 -mbig -mvle
#objdump: -d -Mvle -Mefs2
#name: Validate EFS2 instructions

.*: +file format elf.*-powerpc.*

Disassembly of section .text:

00000000 <.text>:
   0:	10 01 12 b0 	efsmax  r0,r1,r2
   4:	10 01 12 b1 	efsmin  r0,r1,r2
   8:	10 01 12 b8 	efdmax  r0,r1,r2
   c:	10 01 12 b9 	efdmin  r0,r1,r2
  10:	10 01 02 c7 	efssqrt r0,r1
  14:	10 04 12 d1 	efscfh  r0,r2
  18:	10 04 12 d5 	efscth  r0,r2
  1c:	10 01 02 e7 	efdsqrt r0,r1
  20:	10 04 12 f1 	efdcfh  r0,r2
  24:	10 04 12 f5 	efdcth  r0,r2
