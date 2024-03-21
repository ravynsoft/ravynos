#source: ./dcmp.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	76 90 08 10                   	dcmpun	dr0, dr0
   4:	76 90 f8 10                   	dcmpun	dr0, dr15
   8:	76 90 08 1f                   	dcmpun	dr15, dr0
   c:	76 90 f8 1f                   	dcmpun	dr15, dr15
  10:	76 90 08 20                   	dcmpeq	dr0, dr0
  14:	76 90 f8 20                   	dcmpeq	dr0, dr15
  18:	76 90 08 2f                   	dcmpeq	dr15, dr0
  1c:	76 90 f8 2f                   	dcmpeq	dr15, dr15
  20:	76 90 08 40                   	dcmplt	dr0, dr0
  24:	76 90 f8 40                   	dcmplt	dr0, dr15
  28:	76 90 08 4f                   	dcmplt	dr15, dr0
  2c:	76 90 f8 4f                   	dcmplt	dr15, dr15
  30:	76 90 08 60                   	dcmple	dr0, dr0
  34:	76 90 f8 60                   	dcmple	dr0, dr15
  38:	76 90 08 6f                   	dcmple	dr15, dr0
  3c:	76 90 f8 6f                   	dcmple	dr15, dr15
