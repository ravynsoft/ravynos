#as: -a32 -mbig -mvle
#objdump: -dr -Mvle
#name: VLE Simplified mnemonics 5

.*: +file format elf.*-powerpc.*

Disassembly of section .text:

0+0 <.text>:
   0:	74 42 00 01 	e_clrrwi r2,r2,31
   4:	74 62 7d bf 	e_rlwinm r2,r3,15,22,31
   8:	74 a4 f8 48 	e_rlwimi r4,r5,31,1,4
   c:	74 e6 c9 4c 	e_rlwimi r6,r7,25,5,6
  10:	74 41 50 3f 	e_rotlwi r1,r2,10
  14:	74 83 c0 3f 	e_rotlwi r3,r4,24
  18:	7c 62 f8 70 	e_slwi  r2,r3,31
  1c:	7c 25 f4 70 	e_srwi  r5,r1,30
  20:	74 64 07 7f 	e_clrlwi r4,r3,29
  24:	74 41 00 07 	e_clrrwi r1,r2,28
  28:	74 e6 d8 49 	e_rlwinm r6,r7,27,1,4
