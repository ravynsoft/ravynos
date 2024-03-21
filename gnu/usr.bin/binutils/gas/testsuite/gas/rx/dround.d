#source: ./dround.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	76 90 0d 0d                   	dround	dr0, dr0
   4:	76 90 0d fd                   	dround	dr0, dr15
   8:	76 90 fd 0d                   	dround	dr15, dr0
   c:	76 90 fd fd                   	dround	dr15, dr15
