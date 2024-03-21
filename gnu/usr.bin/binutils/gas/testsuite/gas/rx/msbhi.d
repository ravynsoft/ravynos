#source: ./msbhi.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 44 00                      	msbhi	r0, r0, a0
   3:	fd 44 0f                      	msbhi	r0, r15, a0
   6:	fd 44 f0                      	msbhi	r15, r0, a0
   9:	fd 44 ff                      	msbhi	r15, r15, a0
   c:	fd 4c 00                      	msbhi	r0, r0, a1
   f:	fd 4c 0f                      	msbhi	r0, r15, a1
  12:	fd 4c f0                      	msbhi	r15, r0, a1
  15:	fd 4c ff                      	msbhi	r15, r15, a1
