#as: -a32 -mbig -mvle
#objdump: -dr -Mvle
#name: VLE Simplified mnemonics 3

.*: +file format elf.*-powerpc.*

Disassembly of section .text:

0+0 <trap>:
   0:	7f e0 00 08 	trap
   4:	7e 01 10 08 	twlt    r1,r2
   8:	7e 83 20 08 	twle    r3,r4
   c:	7c 80 08 08 	tweq    r0,r1
  10:	7d 82 18 08 	twge    r2,r3
  14:	7d 02 20 08 	twgt    r2,r4
  18:	7d 82 28 08 	twge    r2,r5
  1c:	7f 02 30 08 	twne    r2,r6
  20:	7e 82 38 08 	twle    r2,r7
  24:	7c 42 40 08 	twllt   r2,r8
  28:	7c c2 48 08 	twlle   r2,r9
  2c:	7c a2 50 08 	twlge   r2,r10
  30:	7c 22 58 08 	twlgt   r2,r11
  34:	7c a2 60 08 	twlge   r2,r12
  38:	7c c2 68 08 	twlle   r2,r13
