#source: ./dtoX.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	76 90 0d 0c                   	dtof	dr0, dr0
   4:	76 90 0d fc                   	dtof	dr0, dr15
   8:	76 90 fd 0c                   	dtof	dr15, dr0
   c:	76 90 fd fc                   	dtof	dr15, dr15
  10:	76 90 0d 08                   	dtoi	dr0, dr0
  14:	76 90 0d f8                   	dtoi	dr0, dr15
  18:	76 90 fd 08                   	dtoi	dr15, dr0
  1c:	76 90 fd f8                   	dtoi	dr15, dr15
  20:	76 90 0d 09                   	dtou	dr0, dr0
  24:	76 90 0d f9                   	dtou	dr0, dr15
  28:	76 90 fd 09                   	dtou	dr15, dr0
  2c:	76 90 fd f9                   	dtou	dr15, dr15
