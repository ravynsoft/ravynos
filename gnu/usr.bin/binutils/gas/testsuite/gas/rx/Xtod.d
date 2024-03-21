#source: ./Xtod.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	fd 77 80 0a                   	ftod	r0, dr0
   4:	fd 77 8f 0a                   	ftod	r15, dr0
   8:	fd 77 80 fa                   	ftod	r0, dr15
   c:	fd 77 8f fa                   	ftod	r15, dr15
  10:	fd 77 80 09                   	itod	r0, dr0
  14:	fd 77 8f 09                   	itod	r15, dr0
  18:	fd 77 80 f9                   	itod	r0, dr15
  1c:	fd 77 8f f9                   	itod	r15, dr15
  20:	fd 77 80 0d                   	utod	r0, dr0
  24:	fd 77 8f 0d                   	utod	r15, dr0
  28:	fd 77 80 fd                   	utod	r0, dr15
  2c:	fd 77 8f fd                   	utod	r15, dr15
