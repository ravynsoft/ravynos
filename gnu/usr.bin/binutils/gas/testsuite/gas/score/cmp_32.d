#as:  -march=score3 -I${srcdir}/${subdir}
#objdump:  -d
#source:  cmp_32.s

.*:     file format .*

Disassembly of section .text:

00000000 <.text>:
   0:	440f      	cmp!		r0, r15
   2:	440f      	cmp!		r0, r15
   4:	440f      	cmp!		r0, r15
   6:	440f      	cmp!		r0, r15
   8:	440f      	cmp!		r0, r15
   a:	440f      	cmp!		r0, r15
   c:	440f      	cmp!		r0, r15
   e:	440f      	cmp!		r0, r15
  10:	440f      	cmp!		r0, r15
  12:	4410      	cmp!		r0, r16
  14:	4600      	cmp!		r16, r0
  16:	461f      	cmp!		r16, r31
  18:	6010      	cmpi!		r0, -16
  1a:	600f      	cmpi!		r0, 15
  1c:	61f0      	cmpi!		r15, -16
  1e:	61ef      	cmpi!		r15, 15
  20:	6010      	cmpi!		r0, -16
  22:	6010      	cmpi!		r0, -16
  24:	6010      	cmpi!		r0, -16
  26:	6010      	cmpi!		r0, -16
  28:	6010      	cmpi!		r0, -16
  2a:	6010      	cmpi!		r0, -16
  2c:	6010      	cmpi!		r0, -16
  2e:	6010      	cmpi!		r0, -16
  30:	6210      	cmpi!		r16, -16
  32:	63ef      	cmpi!		r31, 15
  34:	840b 7fdf 	cmpi.c		r0, -17
  38:	85e8 0021 	cmpi.c		r15, 16
  3c:	8608 0021 	cmpi.c		r16, 16
#pass
