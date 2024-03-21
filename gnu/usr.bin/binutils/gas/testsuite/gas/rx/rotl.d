#source: ./rotl.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 6e 00                      	rotl	#0, r0
   3:	fd 6e 0f                      	rotl	#0, r15
   6:	fd 6f f0                      	rotl	#31, r0
   9:	fd 6f ff                      	rotl	#31, r15
   c:	fd 66 00                      	rotl	r0, r0
   f:	fd 66 0f                      	rotl	r0, r15
  12:	fd 66 f0                      	rotl	r15, r0
  15:	fd 66 ff                      	rotl	r15, r15
