#as: -a32 -mbig -mvle
#objdump: -dr -Mvle
#name: VLE Simplified mnemonics 4

.*: +file format elf.*-powerpc.*

Disassembly of section .text:

0+0 <subtract>:
   0:	7c 23 10 50 	subf    r1,r3,r2
   4:	7c a3 20 51 	subf.   r5,r3,r4
   8:	7c 21 14 50 	subfo   r1,r1,r2
   c:	7c 01 14 51 	subfo.  r0,r1,r2
  10:	7c 65 20 10 	subfc   r3,r5,r4
  14:	7c 65 20 11 	subfc.  r3,r5,r4
  18:	7c 23 14 10 	subfco  r1,r3,r2
  1c:	7c a7 34 11 	subfco. r5,r7,r6
  20:	18 85 84 d0 	e_addi  r4,r5,-48
  24:	18 66 94 fe 	e_addic r3,r6,-2
  28:	18 e8 9c f0 	e_addic. r7,r8,-16
  2c:	1c 22 ff f1 	e_add16i r1,r2,-15
  30:	73 e5 8f ff 	e_add2i. r5,-1
  34:	73 ea 97 00 	e_add2is r10,-256
