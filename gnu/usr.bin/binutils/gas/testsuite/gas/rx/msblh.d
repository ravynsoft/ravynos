#source: ./msblh.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 46 00                      	msblh	r0, r0, a0
   3:	fd 46 0f                      	msblh	r0, r15, a0
   6:	fd 46 f0                      	msblh	r15, r0, a0
   9:	fd 46 ff                      	msblh	r15, r15, a0
   c:	fd 4e 00                      	msblh	r0, r0, a1
   f:	fd 4e 0f                      	msblh	r0, r15, a1
  12:	fd 4e f0                      	msblh	r15, r0, a1
  15:	fd 4e ff                      	msblh	r15, r15, a1
