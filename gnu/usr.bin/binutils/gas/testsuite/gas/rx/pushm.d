#source: ./pushm.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	6e 18                         	pushm	r1-r8
   2:	6e 1e                         	pushm	r1-r14
   4:	6e 78                         	pushm	r7-r8
   6:	6e 7e                         	pushm	r7-r14
   8:	7e a4                         	push.l	r4
