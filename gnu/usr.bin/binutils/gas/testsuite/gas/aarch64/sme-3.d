#name: SME extension, MOVA (vector to tile)
#as: -march=armv8-a+sme
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	c0008000 	mov	za0v\.b\[w12, 0\], p0/m, z0\.b
   4:	c0408000 	mov	za0v\.h\[w12, 0\], p0/m, z0\.h
   8:	c0808000 	mov	za0v\.s\[w12, 0\], p0/m, z0\.s
   c:	c0c08000 	mov	za0v\.d\[w12, 0\], p0/m, z0\.d
  10:	c0c18000 	mov	za0v\.q\[w12, 0\], p0/m, z0\.q
  14:	c000ffef 	mov	za0v\.b\[w15, 15\], p7/m, z31\.b
  18:	c040ffef 	mov	za1v\.h\[w15, 7\], p7/m, z31\.h
  1c:	c080ffef 	mov	za3v\.s\[w15, 3\], p7/m, z31\.s
  20:	c0c0ffef 	mov	za7v\.d\[w15, 1\], p7/m, z31\.d
  24:	c0c1ffef 	mov	za15v\.q\[w15, 0\], p7/m, z31\.q
  28:	c0000000 	mov	za0h\.b\[w12, 0\], p0/m, z0\.b
  2c:	c0400000 	mov	za0h\.h\[w12, 0\], p0/m, z0\.h
  30:	c0800000 	mov	za0h\.s\[w12, 0\], p0/m, z0\.s
  34:	c0c00000 	mov	za0h\.d\[w12, 0\], p0/m, z0\.d
  38:	c0c10000 	mov	za0h\.q\[w12, 0\], p0/m, z0\.q
  3c:	c0007fef 	mov	za0h\.b\[w15, 15\], p7/m, z31\.b
  40:	c0407fef 	mov	za1h\.h\[w15, 7\], p7/m, z31\.h
  44:	c0807fef 	mov	za3h\.s\[w15, 3\], p7/m, z31\.s
  48:	c0c07fef 	mov	za7h\.d\[w15, 1\], p7/m, z31\.d
  4c:	c0c17fef 	mov	za15h\.q\[w15, 0\], p7/m, z31\.q
  50:	c0008000 	mov	za0v\.b\[w12, 0\], p0/m, z0\.b
  54:	c0c17fef 	mov	za15h\.q\[w15, 0\], p7/m, z31\.q
