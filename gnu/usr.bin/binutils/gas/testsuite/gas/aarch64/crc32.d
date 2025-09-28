#objdump: -dr
#as: -march=armv8-a+crc

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	1acf40e3 	crc32b	w3, w7, w15
   4:	1ac345e7 	crc32h	w7, w15, w3
   8:	1ac7486f 	crc32w	w15, w3, w7
   c:	9acf4ce3 	crc32x	w3, w7, x15
  10:	1acf50e3 	crc32cb	w3, w7, w15
  14:	1ac355e7 	crc32ch	w7, w15, w3
  18:	1ac7586f 	crc32cw	w15, w3, w7
  1c:	9acf5ce3 	crc32cx	w3, w7, x15
