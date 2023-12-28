#source: ./maclo.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 05 00                      	maclo	r0, r0, a0
   3:	fd 05 0f                      	maclo	r0, r15, a0
   6:	fd 05 f0                      	maclo	r15, r0, a0
   9:	fd 05 ff                      	maclo	r15, r15, a0
   c:	fd 05 00                      	maclo	r0, r0, a0
   f:	fd 05 0f                      	maclo	r0, r15, a0
  12:	fd 05 f0                      	maclo	r15, r0, a0
  15:	fd 05 ff                      	maclo	r15, r15, a0
  18:	fd 0d 00                      	maclo	r0, r0, a1
  1b:	fd 0d 0f                      	maclo	r0, r15, a1
  1e:	fd 0d f0                      	maclo	r15, r0, a1
  21:	fd 0d ff                      	maclo	r15, r15, a1
