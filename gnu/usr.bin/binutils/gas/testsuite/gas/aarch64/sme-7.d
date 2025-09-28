#name: SME extension (LDR and STR instructions)
#as: -march=armv8-a+sme
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	e1000000 	ldr	za\[w12, 0\], \[x0\]
   4:	e10003e0 	ldr	za\[w12, 0\], \[sp\]
   8:	e1000000 	ldr	za\[w12, 0\], \[x0\]
   c:	e10003e0 	ldr	za\[w12, 0\], \[sp\]
  10:	e1006220 	ldr	za\[w15, 0\], \[x17\]
  14:	e1002229 	ldr	za\[w13, 9\], \[x17, #9, mul vl\]
  18:	e100622f 	ldr	za\[w15, 15\], \[x17, #15, mul vl\]
  1c:	e10063ef 	ldr	za\[w15, 15\], \[sp, #15, mul vl\]
  20:	e1200000 	str	za\[w12, 0\], \[x0\]
  24:	e12003e0 	str	za\[w12, 0\], \[sp\]
  28:	e1200000 	str	za\[w12, 0\], \[x0\]
  2c:	e12003e0 	str	za\[w12, 0\], \[sp\]
  30:	e1206220 	str	za\[w15, 0\], \[x17\]
  34:	e1202229 	str	za\[w13, 9\], \[x17, #9, mul vl\]
  38:	e120622f 	str	za\[w15, 15\], \[x17, #15, mul vl\]
  3c:	e12063ef 	str	za\[w15, 15\], \[sp, #15, mul vl\]
  40:	e10003e0 	ldr	za\[w12, 0\], \[sp\]
  44:	e1206220 	str	za\[w15, 0\], \[x17\]
