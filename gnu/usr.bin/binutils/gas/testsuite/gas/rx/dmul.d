#source: ./dmul.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	76 90 02 00                   	dmul	dr0, dr0, dr0
   4:	76 90 02 f0                   	dmul	dr0, dr0, dr15
   8:	76 90 f2 00                   	dmul	dr0, dr15, dr0
   c:	76 90 f2 f0                   	dmul	dr0, dr15, dr15
  10:	76 90 02 0f                   	dmul	dr15, dr0, dr0
  14:	76 90 02 ff                   	dmul	dr15, dr0, dr15
  18:	76 90 f2 0f                   	dmul	dr15, dr15, dr0
  1c:	76 90 f2 ff                   	dmul	dr15, dr15, dr15
