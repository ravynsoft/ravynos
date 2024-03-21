#as: -a32 -mbig -mvle -mregnames
#objdump: -dr -Mvle
#name: VLE Instructions for improving interrupt handler efficiency

.*: +file format elf.*-powerpc.*

Disassembly of section .text:

00000000 <prolog>:
   0:	18 01 11 00 	e_stmvgprw 0\(r1\)
   4:	18 22 11 04 	e_stmvsprw 4\(r2\)
   8:	18 83 11 08 	e_stmvsrrw 8\(r3\)
   c:	18 a4 11 0c 	e_stmvcsrrw 12\(r4\)
  10:	18 c5 11 10 	e_stmvdsrrw 16\(r5\)
  14:	18 e6 11 14 	e_stmvmcsrrw 20\(r6\)
00000018 <epilog>:
  18:	18 07 10 18 	e_lmvgprw 24\(r7\)
  1c:	18 28 10 1c 	e_lmvsprw 28\(r8\)
  20:	18 89 10 20 	e_lmvsrrw 32\(r9\)
  24:	18 aa 10 24 	e_lmvcsrrw 36\(r10\)
  28:	18 cb 10 28 	e_lmvdsrrw 40\(r11\)
  2c:	18 ec 10 2c 	e_lmvmcsrrw 44\(r12\)
00000030 <epilog_alt>:
  30:	18 0d 10 30 	e_lmvgprw 48\(r13\)
  34:	18 2e 10 34 	e_lmvsprw 52\(r14\)
  38:	18 8f 10 38 	e_lmvsrrw 56\(r15\)
  3c:	18 b0 10 3c 	e_lmvcsrrw 60\(r16\)
  40:	18 d1 10 40 	e_lmvdsrrw 64\(r17\)
