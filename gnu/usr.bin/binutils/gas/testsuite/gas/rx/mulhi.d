#source: ./mulhi.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 00 00                      	mulhi	r0, r0, a0
   3:	fd 00 0f                      	mulhi	r0, r15, a0
   6:	fd 00 f0                      	mulhi	r15, r0, a0
   9:	fd 00 ff                      	mulhi	r15, r15, a0
   c:	fd 00 00                      	mulhi	r0, r0, a0
   f:	fd 00 0f                      	mulhi	r0, r15, a0
  12:	fd 00 f0                      	mulhi	r15, r0, a0
  15:	fd 00 ff                      	mulhi	r15, r15, a0
  18:	fd 08 00                      	mulhi	r0, r0, a1
  1b:	fd 08 0f                      	mulhi	r0, r15, a1
  1e:	fd 08 f0                      	mulhi	r15, r0, a1
  21:	fd 08 ff                      	mulhi	r15, r15, a1
