#source: ./abs.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	7e 20                         	abs	r0
   2:	7e 2f                         	abs	r15
   4:	fc 0f 00                      	abs	r0, r0
   7:	fc 0f 0f                      	abs	r0, r15
   a:	fc 0f f0                      	abs	r15, r0
   d:	fc 0f ff                      	abs	r15, r15
