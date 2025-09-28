#source: ./dadd.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	76 90 00 00                   	dadd	dr0, dr0, dr0
   4:	76 90 00 f0                   	dadd	dr0, dr0, dr15
   8:	76 90 f0 00                   	dadd	dr0, dr15, dr0
   c:	76 90 f0 f0                   	dadd	dr0, dr15, dr15
  10:	76 90 00 0f                   	dadd	dr15, dr0, dr0
  14:	76 90 00 ff                   	dadd	dr15, dr0, dr15
  18:	76 90 f0 0f                   	dadd	dr15, dr15, dr0
  1c:	76 90 f0 ff                   	dadd	dr15, dr15, dr15

