#source: ./rstr.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	fd 76 d0 00                   	rstr	r0
   4:	fd 76 df 00                   	rstr	r15
   8:	fd 76 f0 00                   	rstr	#0
   c:	fd 76 f0 ff                   	rstr	#255
