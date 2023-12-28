#as: -Av8
#objdump: -dr -m sparc
#warning: Warning: FP branch preceded by FP compare; NOP inserted
#name: v8 branch instructions

.*: +file format .*

Disassembly of section .text:

0+ <no_fpop2_before_fcmp>:
   0:	81 a0 08 20 	fadds  %f0, %f0, %f0
   4:	13 80 00 06 	fbe  1c <no_fpop2_before_fcmp\+0x1c>
   8:	01 00 00 00 	nop 
   c:	81 a8 0a 20 	fcmps  %f0, %f0
  10:	01 00 00 00 	nop 
  14:	13 80 00 02 	fbe  1c <no_fpop2_before_fcmp\+0x1c>
  18:	01 00 00 00 	nop 
  1c:	01 00 00 00 	nop 
