#source: ./mullh.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 02 00                      	mullh	r0, r0, a0
   3:	fd 02 0f                      	mullh	r0, r15, a0
   6:	fd 02 f0                      	mullh	r15, r0, a0
   9:	fd 02 ff                      	mullh	r15, r15, a0
   c:	fd 0a 00                      	mullh	r0, r0, a1
   f:	fd 0a 0f                      	mullh	r0, r15, a1
  12:	fd 0a f0                      	mullh	r15, r0, a1
  15:	fd 0a ff                      	mullh	r15, r15, a1
