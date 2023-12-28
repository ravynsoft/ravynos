#source: ./maclh.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 06 00                      	maclh	r0, r0, a0
   3:	fd 06 0f                      	maclh	r0, r15, a0
   6:	fd 06 f0                      	maclh	r15, r0, a0
   9:	fd 06 ff                      	maclh	r15, r15, a0
   c:	fd 0e 00                      	maclh	r0, r0, a1
   f:	fd 0e 0f                      	maclh	r0, r15, a1
  12:	fd 0e f0                      	maclh	r15, r0, a1
  15:	fd 0e ff                      	maclh	r15, r15, a1
