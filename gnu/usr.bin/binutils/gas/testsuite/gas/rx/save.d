#source: ./save.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	fd 76 c0 00                   	save	r0
   4:	fd 76 cf 00                   	save	r15
   8:	fd 76 e0 00                   	save	#0
   c:	fd 76 e0 ff                   	save	#255
