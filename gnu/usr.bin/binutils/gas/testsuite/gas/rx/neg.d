#source: ./neg.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	7e 10                         	neg	r0
   2:	7e 1f                         	neg	r15
   4:	fc 07 00                      	neg	r0, r0
   7:	fc 07 0f                      	neg	r0, r15
   a:	fc 07 f0                      	neg	r15, r0
   d:	fc 07 ff                      	neg	r15, r15
