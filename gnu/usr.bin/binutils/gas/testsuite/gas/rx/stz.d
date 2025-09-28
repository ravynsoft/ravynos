#source: ./stz.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 74 e0 80                   	stz	#-128, r0
   4:	fd 74 ef 80                   	stz	#-128, r15
   8:	fd 74 e0 7f                   	stz	#127, r0
   c:	fd 74 ef 7f                   	stz	#127, r15
  10:	fd 78 e0 00 80                	stz	#0xffff8000, r0
  15:	fd 78 ef 00 80                	stz	#0xffff8000, r15
  1a:	fd 7c e0 00 80 00             	stz	#0x8000, r0
  20:	fd 7c ef 00 80 00             	stz	#0x8000, r15
  26:	fd 7c e0 00 00 80             	stz	#0xff800000, r0
  2c:	fd 7c ef 00 00 80             	stz	#0xff800000, r15
  32:	fd 7c e0 ff ff 7f             	stz	#0x7fffff, r0
  38:	fd 7c ef ff ff 7f             	stz	#0x7fffff, r15
  3e:	fd 70 e0 00 00 00 80          	stz	#0x80000000, r0
  45:	fd 70 ef 00 00 00 80          	stz	#0x80000000, r15
  4c:	fd 70 e0 ff ff ff 7f          	stz	#0x7fffffff, r0
  53:	fd 70 ef ff ff ff 7f          	stz	#0x7fffffff, r15
  5a:	fc 4b 00                      	stz	r0, r0
  5d:	fc 4b 0f                      	stz	r0, r15
  60:	fc 4b f0                      	stz	r15, r0
  63:	fc 4b ff                      	stz	r15, r15
