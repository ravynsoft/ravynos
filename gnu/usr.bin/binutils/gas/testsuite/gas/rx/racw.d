#source: ./racw.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 18 00                      	racw	#1, a0
   3:	fd 18 10                      	racw	#2, a0
   6:	fd 18 00                      	racw	#1, a0
   9:	fd 18 10                      	racw	#2, a0
   c:	fd 18 80                      	racw	#1, a1
   f:	fd 18 90                      	racw	#2, a1
