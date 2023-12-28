#source: ./racl.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 19 00                      	racl	#1, a0
   3:	fd 19 10                      	racl	#2, a0
   6:	fd 19 80                      	racl	#1, a1
   9:	fd 19 90                      	racl	#2, a1
