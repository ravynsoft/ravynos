#source: ./emula.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 03 00                      	emula	r0, r0, a0
   3:	fd 03 0f                      	emula	r0, r15, a0
   6:	fd 03 f0                      	emula	r15, r0, a0
   9:	fd 03 ff                      	emula	r15, r15, a0
   c:	fd 0b 00                      	emula	r0, r0, a1
   f:	fd 0b 0f                      	emula	r0, r15, a1
  12:	fd 0b f0                      	emula	r15, r0, a1
  15:	fd 0b ff                      	emula	r15, r15, a1
