#source: ./not.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	7e 00                         	not	r0
   2:	7e 0f                         	not	r15
   4:	fc 3b 00                      	not	r0, r0
   7:	fc 3b 0f                      	not	r0, r15
   a:	fc 3b f0                      	not	r15, r0
   d:	fc 3b ff                      	not	r15, r15
