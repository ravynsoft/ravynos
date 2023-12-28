#objdump: -dr
#as: --defsym DIRECTIVE=1
#source: lor.s

.*:     file format .*


Disassembly of section \.text:

0+ <.text>:
   0:	889f7c00 	stllr	w0, \[x0\]
   4:	c89f7c00 	stllr	x0, \[x0\]
   8:	889f7c01 	stllr	w1, \[x0\]
   c:	c89f7c22 	stllr	x2, \[x1\]
  10:	489f7c43 	stllrh	w3, \[x2\]
  14:	089f7c64 	stllrb	w4, \[x3\]
  18:	089f7fe5 	stllrb	w5, \[sp\]
  1c:	88df7c00 	ldlar	w0, \[x0\]
  20:	c8df7c00 	ldlar	x0, \[x0\]
  24:	88df7c01 	ldlar	w1, \[x0\]
  28:	c8df7c22 	ldlar	x2, \[x1\]
  2c:	08df7c43 	ldlarb	w3, \[x2\]
  30:	48df7c64 	ldlarh	w4, \[x3\]
  34:	88df7fe5 	ldlar	w5, \[sp\]

