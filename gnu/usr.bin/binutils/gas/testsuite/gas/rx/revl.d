#source: ./revl.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 67 00                      	revl	r0, r0
   3:	fd 67 0f                      	revl	r0, r15
   6:	fd 67 f0                      	revl	r15, r0
   9:	fd 67 ff                      	revl	r15, r15
