#source: ./shar.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	6a 00                         	shar	#0, r0
   2:	6a 0f                         	shar	#0, r15
   4:	6b f0                         	shar	#31, r0
   6:	6b ff                         	shar	#31, r15
   8:	fd 61 00                      	shar	r0, r0
   b:	fd 61 0f                      	shar	r0, r15
   e:	fd 61 f0                      	shar	r15, r0
  11:	fd 61 ff                      	shar	r15, r15
  14:	fd a0 00                      	shar	#0, r0, r0
  17:	fd a0 0f                      	shar	#0, r0, r15
  1a:	fd a0 f0                      	shar	#0, r15, r0
  1d:	fd a0 ff                      	shar	#0, r15, r15
  20:	fd bf 00                      	shar	#31, r0, r0
  23:	fd bf 0f                      	shar	#31, r0, r15
  26:	fd bf f0                      	shar	#31, r15, r0
  29:	fd bf ff                      	shar	#31, r15, r15
