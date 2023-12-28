#source: ./dabs.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	76 90 0c 01                   	dabs	dr0, dr0
   4:	76 90 0c f1                   	dabs	dr0, dr15
   8:	76 90 fc 01                   	dabs	dr15, dr0
   c:	76 90 fc f1                   	dabs	dr15, dr15

