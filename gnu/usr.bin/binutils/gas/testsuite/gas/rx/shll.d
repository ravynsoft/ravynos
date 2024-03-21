#source: ./shll.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	6c 00                         	shll	#0, r0
   2:	6c 0f                         	shll	#0, r15
   4:	6d f0                         	shll	#31, r0
   6:	6d ff                         	shll	#31, r15
   8:	fd 62 00                      	shll	r0, r0
   b:	fd 62 0f                      	shll	r0, r15
   e:	fd 62 f0                      	shll	r15, r0
  11:	fd 62 ff                      	shll	r15, r15
  14:	fd c0 00                      	shll	#0, r0, r0
  17:	fd c0 0f                      	shll	#0, r0, r15
  1a:	fd c0 f0                      	shll	#0, r15, r0
  1d:	fd c0 ff                      	shll	#0, r15, r15
  20:	fd df 00                      	shll	#31, r0, r0
  23:	fd df 0f                      	shll	#31, r0, r15
  26:	fd df f0                      	shll	#31, r15, r0
  29:	fd df ff                      	shll	#31, r15, r15
