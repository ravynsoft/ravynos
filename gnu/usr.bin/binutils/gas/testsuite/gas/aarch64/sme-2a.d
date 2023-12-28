#name: SME extension, MOV (tile to vector)
#as: -march=armv8-a+sme
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	c0028000 	mov	z0\.b, p0/m, za0v\.b\[w12, 0\]
   4:	c0428000 	mov	z0\.h, p0/m, za0v\.h\[w12, 0\]
   8:	c0828000 	mov	z0\.s, p0/m, za0v\.s\[w12, 0\]
   c:	c0c28000 	mov	z0\.d, p0/m, za0v\.d\[w12, 0\]
  10:	c0c38000 	mov	z0\.q, p0/m, za0v\.q\[w12, 0\]
  14:	c002fdff 	mov	z31\.b, p7/m, za0v\.b\[w15, 15\]
  18:	c042fdff 	mov	z31\.h, p7/m, za1v\.h\[w15, 7\]
  1c:	c082fdff 	mov	z31\.s, p7/m, za3v\.s\[w15, 3\]
  20:	c0c2fdff 	mov	z31\.d, p7/m, za7v\.d\[w15, 1\]
  24:	c0c3fdff 	mov	z31\.q, p7/m, za15v\.q\[w15, 0\]
  28:	c0020000 	mov	z0\.b, p0/m, za0h\.b\[w12, 0\]
  2c:	c0420000 	mov	z0\.h, p0/m, za0h\.h\[w12, 0\]
  30:	c0820000 	mov	z0\.s, p0/m, za0h\.s\[w12, 0\]
  34:	c0c20000 	mov	z0\.d, p0/m, za0h\.d\[w12, 0\]
  38:	c0c30000 	mov	z0\.q, p0/m, za0h\.q\[w12, 0\]
  3c:	c0027dff 	mov	z31\.b, p7/m, za0h\.b\[w15, 15\]
  40:	c0427dff 	mov	z31\.h, p7/m, za1h\.h\[w15, 7\]
  44:	c0827dff 	mov	z31\.s, p7/m, za3h\.s\[w15, 3\]
  48:	c0c27dff 	mov	z31\.d, p7/m, za7h\.d\[w15, 1\]
  4c:	c0c37dff 	mov	z31\.q, p7/m, za15h\.q\[w15, 0\]
