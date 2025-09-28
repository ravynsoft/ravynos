#source: ./nop.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	03                            	nop
   1:	ef 00                         	nop	; mov.l	r0, r0
   3:	63 10                         	nop	; mul	#1, r0
   5:	fc 13 00                      	nop	; max	r0, r0
   8:	76 10 01 00                   	nop	; mul	#1, r0
   c:	77 10 01 00 00                	nop	; mul	#1, r0
  11:	74 10 01 00 00 00             	nop	; mul	#1, r0
  17:	fd 70 40 00 00 00 80          	nop	; max	#0x80000000, r0
