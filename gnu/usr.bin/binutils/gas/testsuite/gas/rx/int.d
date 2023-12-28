#source: ./int.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	75 60 00                      	int #0
   3:	75 60 ff                      	int #255
