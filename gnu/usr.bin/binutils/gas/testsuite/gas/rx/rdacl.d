#source: ./rdacl.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 19 40                      	rdacl	#1, a0
   3:	fd 19 50                      	rdacl	#2, a0
   6:	fd 19 c0                      	rdacl	#1, a1
   9:	fd 19 d0                      	rdacl	#2, a1
