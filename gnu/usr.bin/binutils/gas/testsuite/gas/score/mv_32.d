#as:  -march=score3 -I${srcdir}/${subdir}
#objdump:  -d
#source:  mv_32.s

.*:     file format .*

Disassembly of section .text:

00000000 <.text>:
   0:	400f      	mv!		r0, r15
   2:	400f      	mv!		r0, r15
   4:	400f      	mv!		r0, r15
   6:	400f      	mv!		r0, r15
   8:	400f      	mv!		r0, r15
   a:	400f      	mv!		r0, r15
   c:	400f      	mv!		r0, r15
   e:	400f      	mv!		r0, r15
  10:	400f      	mv!		r0, r15
  12:	420f      	mv!		r16, r15
  14:	4010      	mv!		r0, r16
  16:	4210      	mv!		r16, r16
#pass
