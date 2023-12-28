#source: ./rdacw.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 18 40                      	rdacw	#1, a0
   3:	fd 18 50                      	rdacw	#2, a0
   6:	fd 18 c0                      	rdacw	#1, a1
   9:	fd 18 d0                      	rdacw	#2, a1
