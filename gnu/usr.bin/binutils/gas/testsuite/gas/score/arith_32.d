#as:  -march=score3 -I${srcdir}/${subdir}
#objdump:  -d
#source:  arith_32.s

.*:     file format .*

Disassembly of section .text:

00000000 <.text>:
   0:	480f      	add!		r0, r15
   2:	480f      	add!		r0, r15
   4:	480f      	add!		r0, r15
   6:	480f      	add!		r0, r15
   8:	480f      	add!		r0, r15
   a:	480f      	add!		r0, r15
   c:	480f      	add!		r0, r15
   e:	480f      	add!		r0, r15
  10:	480f      	add!		r0, r15
  12:	8000 3c11 	add.c		r0, r0, r15
  16:	8000 4010 	add		r0, r0, r16
  1a:	8210 0010 	add		r16, r16, r0
  1e:	8210 4410 	add		r16, r16, r17
  22:	8001 0810 	add		r0, r1, r2
  26:	490f      	sub!		r0, r15
  28:	490f      	sub!		r0, r15
  2a:	490f      	sub!		r0, r15
  2c:	490f      	sub!		r0, r15
  2e:	490f      	sub!		r0, r15
  30:	490f      	sub!		r0, r15
  32:	490f      	sub!		r0, r15
  34:	490f      	sub!		r0, r15
  36:	490f      	sub!		r0, r15
  38:	8000 3c15 	sub.c		r0, r0, r15
  3c:	8000 4014 	sub		r0, r0, r16
  40:	8210 0014 	sub		r16, r16, r0
  44:	8210 4414 	sub		r16, r16, r17
  48:	8001 0814 	sub		r0, r1, r2
  4c:	5c20      	addi!		r0, -32
  4e:	5c1f      	addi!		r0, 31
  50:	5fe0      	addi!		r15, -32
  52:	5fdf      	addi!		r15, 31
  54:	5c20      	addi!		r0, -32
  56:	5c20      	addi!		r0, -32
  58:	5c20      	addi!		r0, -32
  5a:	5c20      	addi!		r0, -32
  5c:	5c20      	addi!		r0, -32
  5e:	5c20      	addi!		r0, -32
  60:	5c20      	addi!		r0, -32
  62:	5c20      	addi!		r0, -32
  64:	8403 7fc1 	addi.c		r0, -32
  68:	8403 7fbe 	addi		r0, -33
  6c:	8400 0040 	addi		r0, 32
  70:	8603 7fc0 	addi		r16, -32
  74:	8600 003e 	addi		r16, 31
#pass
