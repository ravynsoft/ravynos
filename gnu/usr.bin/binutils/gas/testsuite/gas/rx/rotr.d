#source: ./rotr.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 6c 00                      	rotr	#0, r0
   3:	fd 6c 0f                      	rotr	#0, r15
   6:	fd 6d f0                      	rotr	#31, r0
   9:	fd 6d ff                      	rotr	#31, r15
   c:	fd 64 00                      	rotr	r0, r0
   f:	fd 64 0f                      	rotr	r0, r15
  12:	fd 64 f0                      	rotr	r15, r0
  15:	fd 64 ff                      	rotr	r15, r15
