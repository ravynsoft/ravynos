#objdump: -d
#name:    
#source:  mul-reg.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	4c a8       	muls d0, d1, d2
   2:	4d 81       	muls d1, d2, d3
   4:	48 8a       	muls d2, d3, d4
   6:	49 93       	muls d3, d4, d5
   8:	4a 9e       	muls d4, d5, d6
   a:	4b b7       	muls d5, d6, d7
   c:	4e bc       	muls d6, d7, d0
   e:	4f a5       	muls d7, d0, d1
  10:	4c 28       	mulu d0, d1, d2
  12:	4d 01       	mulu d1, d2, d3
  14:	48 0a       	mulu d2, d3, d4
  16:	49 13       	mulu d3, d4, d5
  18:	4a 1e       	mulu d4, d5, d6
  1a:	4b 37       	mulu d5, d6, d7
  1c:	4e 3c       	mulu d6, d7, d0
  1e:	4f 25       	mulu d7, d0, d1
