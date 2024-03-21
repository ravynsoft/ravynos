#as:  -march=score3 -I${srcdir}/${subdir}
#objdump:  -d
#source:  load_store_32.s

.*:     file format .*

Disassembly of section .text:

00000000 <.text>:
   0:	1000      	lw!		r0, \[r0,0\]
   2:	1f00      	lw!		r15, \[r0,0\]
   4:	10e0      	lw!		r0, \[r7,0\]
   6:	1fe0      	lw!		r15, \[r7,0\]
   8:	101f      	lw!		r0, \[r0,124\]
   a:	1f1f      	lw!		r15, \[r0,124\]
   c:	10ff      	lw!		r0, \[r7,124\]
   e:	1fff      	lw!		r15, \[r7,124\]
  10:	101f      	lw!		r0, \[r0,124\]
  12:	101f      	lw!		r0, \[r0,124\]
  14:	101f      	lw!		r0, \[r0,124\]
  16:	101f      	lw!		r0, \[r0,124\]
  18:	101f      	lw!		r0, \[r0,124\]
  1a:	101f      	lw!		r0, \[r0,124\]
  1c:	101f      	lw!		r0, \[r0,124\]
  1e:	101f      	lw!		r0, \[r0,124\]
  20:	c200 0000 	lw		r16, \[r0, 0\]
  24:	c008 007c 	lw		r0, \[r8, 124\]
  28:	c208 007c 	lw		r16, \[r8, 124\]
  2c:	c007 7fff 	lw		r0, \[r7, -1\]
  30:	c007 0080 	lw		r0, \[r7, 128\]
  34:	2000      	sw!		r0, \[r0,0\]
  36:	2f00      	sw!		r15, \[r0,0\]
  38:	20e0      	sw!		r0, \[r7,0\]
  3a:	2fe0      	sw!		r15, \[r7,0\]
  3c:	201f      	sw!		r0, \[r0,124\]
  3e:	2f1f      	sw!		r15, \[r0,124\]
  40:	20ff      	sw!		r0, \[r7,124\]
  42:	2fff      	sw!		r15, \[r7,124\]
  44:	201f      	sw!		r0, \[r0,124\]
  46:	201f      	sw!		r0, \[r0,124\]
  48:	201f      	sw!		r0, \[r0,124\]
  4a:	201f      	sw!		r0, \[r0,124\]
  4c:	201f      	sw!		r0, \[r0,124\]
  4e:	201f      	sw!		r0, \[r0,124\]
  50:	201f      	sw!		r0, \[r0,124\]
  52:	201f      	sw!		r0, \[r0,124\]
  54:	d200 0000 	sw		r16, \[r0, 0\]
  58:	d008 007c 	sw		r0, \[r8, 124\]
  5c:	d208 007c 	sw		r16, \[r8, 124\]
  60:	d007 7fff 	sw		r0, \[r7, -1\]
  64:	d007 0080 	sw		r0, \[r7, 128\]
  68:	6400      	ldiu!		r0, 0
  6a:	65e0      	ldiu!		r15, 0
  6c:	641f      	ldiu!		r0, 31
  6e:	65ff      	ldiu!		r15, 31
  70:	6400      	ldiu!		r0, 0
  72:	6400      	ldiu!		r0, 0
  74:	6400      	ldiu!		r0, 0
  76:	6400      	ldiu!		r0, 0
  78:	6400      	ldiu!		r0, 0
  7a:	6400      	ldiu!		r0, 0
  7c:	6400      	ldiu!		r0, 0
  7e:	6400      	ldiu!		r0, 0
  80:	6600      	ldiu!		r16, 0
  82:	841b 7ffe 	ldi		r0, 0xffff\(-1\)
  86:	8418 0040 	ldi		r0, 0x20\(32\)
  8a:	8618 0040 	ldi		r16, 0x20\(32\)
  8e:	0042      	pop!		r2
  90:	004f      	pop!		r15
  92:	0050      	pop!		r16
  94:	9c82 0020 	lw		r4, \[r2\]\+, 4
  98:	9c80 7fe0 	lw		r4, \[r0\]\+, -4
  9c:	0062      	push!		r2
  9e:	006f      	push!		r15
  a0:	0070      	push!		r16
  a2:	8c82 7fe4 	sw		r4, \[r2, -4\]\+
  a6:	8c80 0024 	sw		r4, \[r0, 4\]\+
	...
#pass
