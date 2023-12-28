#source: ./emaca.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 07 00                      	emaca	r0, r0, a0
   3:	fd 07 0f                      	emaca	r0, r15, a0
   6:	fd 07 f0                      	emaca	r15, r0, a0
   9:	fd 07 ff                      	emaca	r15, r15, a0
   c:	fd 0f 00                      	emaca	r0, r0, a1
   f:	fd 0f 0f                      	emaca	r0, r15, a1
  12:	fd 0f f0                      	emaca	r15, r0, a1
  15:	fd 0f ff                      	emaca	r15, r15, a1
