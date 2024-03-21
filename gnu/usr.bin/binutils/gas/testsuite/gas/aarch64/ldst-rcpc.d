#objdump: -dr
#as: -march=armv8.3-a
#as: -march=armv8.2-a+rcpc
#source: ldst-rcpc.s

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	38bfc0e1 	ldaprb	w1, \[x7\]
   4:	38bfc0e1 	ldaprb	w1, \[x7\]
   8:	38bfc0e1 	ldaprb	w1, \[x7\]
   c:	78bfc0e1 	ldaprh	w1, \[x7\]
  10:	78bfc0e1 	ldaprh	w1, \[x7\]
  14:	78bfc0e1 	ldaprh	w1, \[x7\]
  18:	b8bfc0e1 	ldapr	w1, \[x7\]
  1c:	b8bfc0e1 	ldapr	w1, \[x7\]
  20:	b8bfc0e1 	ldapr	w1, \[x7\]
  24:	f8bfc0e1 	ldapr	x1, \[x7\]
  28:	f8bfc0e1 	ldapr	x1, \[x7\]
  2c:	f8bfc0e1 	ldapr	x1, \[x7\]
