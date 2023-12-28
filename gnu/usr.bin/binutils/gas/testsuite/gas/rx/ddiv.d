#source: ./ddiv.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	76 90 05 00                   	ddiv	dr0, dr0, dr0
   4:	76 90 05 f0                   	ddiv	dr0, dr0, dr15
   8:	76 90 f5 00                   	ddiv	dr0, dr15, dr0
   c:	76 90 f5 f0                   	ddiv	dr0, dr15, dr15
  10:	76 90 05 0f                   	ddiv	dr15, dr0, dr0
  14:	76 90 05 ff                   	ddiv	dr15, dr0, dr15
  18:	76 90 f5 0f                   	ddiv	dr15, dr15, dr0
  1c:	76 90 f5 ff                   	ddiv	dr15, dr15, dr15
