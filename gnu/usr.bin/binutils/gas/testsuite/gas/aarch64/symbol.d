#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	b9400401 	ldr	w1, \[x0, #4\]
   4:	b9400401 	ldr	w1, \[x0, #4\]
   8:	b9401001 	ldr	w1, \[x0, #16\]
   c:	b9401001 	ldr	w1, \[x0, #16\]
  10:	8b020020 	add	x0, x1, x2
  14:	91002820 	add	x0, x1, #0xa
  18:	d1002c20 	sub	x0, x1, #0xb
