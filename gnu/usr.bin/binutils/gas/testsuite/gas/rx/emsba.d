#source: ./emsba.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 47 00                      	emsba	r0, r0, a0
   3:	fd 47 0f                      	emsba	r0, r15, a0
   6:	fd 47 f0                      	emsba	r15, r0, a0
   9:	fd 47 ff                      	emsba	r15, r15, a0
   c:	fd 4f 00                      	emsba	r0, r0, a1
   f:	fd 4f 0f                      	emsba	r0, r15, a1
  12:	fd 4f f0                      	emsba	r15, r0, a1
  15:	fd 4f ff                      	emsba	r15, r15, a1
