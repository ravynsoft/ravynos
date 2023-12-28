#as:  -march=score3 -I${srcdir}/${subdir}
#objdump:  -d
#source:  shift_32.s

.*:     file format .*

Disassembly of section .text:

00000000 <.text>:
   0:	5800      	slli!		r0, 0
   2:	581f      	slli!		r0, 31
   4:	59e0      	slli!		r15, 0
   6:	59ff      	slli!		r15, 31
   8:	5800      	slli!		r0, 0
   a:	5800      	slli!		r0, 0
   c:	5800      	slli!		r0, 0
   e:	5800      	slli!		r0, 0
  10:	5800      	slli!		r0, 0
  12:	5800      	slli!		r0, 0
  14:	5800      	slli!		r0, 0
  16:	5800      	slli!		r0, 0
  18:	8000 0071 	slli.c		r0, r0, 0
  1c:	8002 0070 	slli		r0, r2, 0
  20:	8210 0070 	slli		r16, r16, 0
  24:	5a00      	srli!		r0, 0
  26:	5a1f      	srli!		r0, 31
  28:	5be0      	srli!		r15, 0
  2a:	5bff      	srli!		r15, 31
  2c:	5a00      	srli!		r0, 0
  2e:	5a00      	srli!		r0, 0
  30:	5a00      	srli!		r0, 0
  32:	5a00      	srli!		r0, 0
  34:	5a00      	srli!		r0, 0
  36:	5a00      	srli!		r0, 0
  38:	5a00      	srli!		r0, 0
  3a:	5a00      	srli!		r0, 0
  3c:	8000 0075 	srli.c		r0, r0, 0
  40:	8002 0074 	srli		r0, r2, 0
  44:	8210 0074 	srli		r16, r16, 0
#pass
