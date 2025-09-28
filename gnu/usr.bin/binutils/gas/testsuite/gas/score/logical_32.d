#as:  -march=score3 -I${srcdir}/${subdir}
#objdump:  -d
#source:  logical_32.s

.*:     file format .*

Disassembly of section .text:

00000000 <.text>:
   0:	4b0f      	and!		r0, r15
   2:	4b0f      	and!		r0, r15
   4:	4b0f      	and!		r0, r15
   6:	4b0f      	and!		r0, r15
   8:	4b0f      	and!		r0, r15
   a:	4b0f      	and!		r0, r15
   c:	4b0f      	and!		r0, r15
   e:	4b0f      	and!		r0, r15
  10:	4b0f      	and!		r0, r15
  12:	8000 3c21 	and.c		r0, r0, r15
  16:	8000 4020 	and		r0, r0, r16
  1a:	8210 0020 	and		r16, r16, r0
  1e:	8210 4420 	and		r16, r16, r17
  22:	8001 0820 	and		r0, r1, r2
  26:	4a0f      	or!		r0, r15
  28:	4a0f      	or!		r0, r15
  2a:	4a0f      	or!		r0, r15
  2c:	4a0f      	or!		r0, r15
  2e:	4a0f      	or!		r0, r15
  30:	4a0f      	or!		r0, r15
  32:	4a0f      	or!		r0, r15
  34:	4a0f      	or!		r0, r15
  36:	4a0f      	or!		r0, r15
  38:	8000 3c23 	or.c		r0, r0, r15
  3c:	8000 4022 	or		r0, r0, r16
  40:	8210 0022 	or		r16, r16, r0
  44:	8210 4422 	or		r16, r16, r17
  48:	8001 0822 	or		r0, r1, r2
#pass
