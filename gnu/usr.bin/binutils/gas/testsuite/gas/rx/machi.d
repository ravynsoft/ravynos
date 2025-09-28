#source: ./machi.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 04 00                      	machi	r0, r0, a0
   3:	fd 04 0f                      	machi	r0, r15, a0
   6:	fd 04 f0                      	machi	r15, r0, a0
   9:	fd 04 ff                      	machi	r15, r15, a0
   c:	fd 04 00                      	machi	r0, r0, a0
   f:	fd 04 0f                      	machi	r0, r15, a0
  12:	fd 04 f0                      	machi	r15, r0, a0
  15:	fd 04 ff                      	machi	r15, r15, a0
  18:	fd 0c 00                      	machi	r0, r0, a1
  1b:	fd 0c 0f                      	machi	r0, r15, a1
  1e:	fd 0c f0                      	machi	r15, r0, a1
  21:	fd 0c ff                      	machi	r15, r15, a1
