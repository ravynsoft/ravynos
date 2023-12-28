#source: ./shlr.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	68 00                         	shlr	#0, r0
   2:	68 0f                         	shlr	#0, r15
   4:	69 f0                         	shlr	#31, r0
   6:	69 ff                         	shlr	#31, r15
   8:	fd 60 00                      	shlr	r0, r0
   b:	fd 60 0f                      	shlr	r0, r15
   e:	fd 60 f0                      	shlr	r15, r0
  11:	fd 60 ff                      	shlr	r15, r15
  14:	fd 80 00                      	shlr	#0, r0, r0
  17:	fd 80 0f                      	shlr	#0, r0, r15
  1a:	fd 80 f0                      	shlr	#0, r15, r0
  1d:	fd 80 ff                      	shlr	#0, r15, r15
  20:	fd 9f 00                      	shlr	#31, r0, r0
  23:	fd 9f 0f                      	shlr	#31, r0, r15
  26:	fd 9f f0                      	shlr	#31, r15, r0
  29:	fd 9f ff                      	shlr	#31, r15, r15
