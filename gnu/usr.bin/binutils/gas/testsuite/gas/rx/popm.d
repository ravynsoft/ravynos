#source: ./popm.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	6f 18                         	popm	r1-r8
   2:	6f 1e                         	popm	r1-r14
   4:	6f 78                         	popm	r7-r8
   6:	6f 7e                         	popm	r7-r14
   8:	7e b4                         	pop	r4
