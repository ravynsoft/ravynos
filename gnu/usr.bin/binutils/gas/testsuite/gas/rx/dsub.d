#source: ./dsub.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	76 90 01 00                   	dsub	dr0, dr0, dr0
   4:	76 90 01 f0                   	dsub	dr0, dr0, dr15
   8:	76 90 f1 00                   	dsub	dr0, dr15, dr0
   c:	76 90 f1 f0                   	dsub	dr0, dr15, dr15
  10:	76 90 01 0f                   	dsub	dr15, dr0, dr0
  14:	76 90 01 ff                   	dsub	dr15, dr0, dr15
  18:	76 90 f1 0f                   	dsub	dr15, dr15, dr0
  1c:	76 90 f1 ff                   	dsub	dr15, dr15, dr15
