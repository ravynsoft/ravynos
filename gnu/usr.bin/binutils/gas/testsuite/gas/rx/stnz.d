#source: ./stnz.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 74 f0 80                   	stnz	#-128, r0
   4:	fd 74 ff 80                   	stnz	#-128, r15
   8:	fd 74 f0 7f                   	stnz	#127, r0
   c:	fd 74 ff 7f                   	stnz	#127, r15
  10:	fd 78 f0 00 80                	stnz	#0xffff8000, r0
  15:	fd 78 ff 00 80                	stnz	#0xffff8000, r15
  1a:	fd 7c f0 00 80 00             	stnz	#0x8000, r0
  20:	fd 7c ff 00 80 00             	stnz	#0x8000, r15
  26:	fd 7c f0 00 00 80             	stnz	#0xff800000, r0
  2c:	fd 7c ff 00 00 80             	stnz	#0xff800000, r15
  32:	fd 7c f0 ff ff 7f             	stnz	#0x7fffff, r0
  38:	fd 7c ff ff ff 7f             	stnz	#0x7fffff, r15
  3e:	fd 70 f0 00 00 00 80          	stnz	#0x80000000, r0
  45:	fd 70 ff 00 00 00 80          	stnz	#0x80000000, r15
  4c:	fd 70 f0 ff ff ff 7f          	stnz	#0x7fffffff, r0
  53:	fd 70 ff ff ff ff 7f          	stnz	#0x7fffffff, r15
  5a:	fc 4f 00                      	stnz	r0, r0
  5d:	fc 4f 0f                      	stnz	r0, r15
  60:	fc 4f f0                      	stnz	r15, r0
  63:	fc 4f ff                      	stnz	r15, r15
