#source: ./msblo.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 45 00                      	msblo	r0, r0, a0
   3:	fd 45 0f                      	msblo	r0, r15, a0
   6:	fd 45 f0                      	msblo	r15, r0, a0
   9:	fd 45 ff                      	msblo	r15, r15, a0
   c:	fd 4d 00                      	msblo	r0, r0, a1
   f:	fd 4d 0f                      	msblo	r0, r15, a1
  12:	fd 4d f0                      	msblo	r15, r0, a1
  15:	fd 4d ff                      	msblo	r15, r15, a1
