#source: ./mullo.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 01 00                      	mullo	r0, r0, a0
   3:	fd 01 0f                      	mullo	r0, r15, a0
   6:	fd 01 f0                      	mullo	r15, r0, a0
   9:	fd 01 ff                      	mullo	r15, r15, a0
   c:	fd 01 00                      	mullo	r0, r0, a0
   f:	fd 01 0f                      	mullo	r0, r15, a0
  12:	fd 01 f0                      	mullo	r15, r0, a0
  15:	fd 01 ff                      	mullo	r15, r15, a0
  18:	fd 09 00                      	mullo	r0, r0, a1
  1b:	fd 09 0f                      	mullo	r0, r15, a1
  1e:	fd 09 f0                      	mullo	r15, r0, a1
  21:	fd 09 ff                      	mullo	r15, r15, a1
