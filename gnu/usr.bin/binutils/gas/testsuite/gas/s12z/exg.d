#objdump: -d
#name:    
#source:  exg.s
#warning_output: exg.l


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	ae 45       	exg d0, d1
   2:	ae 55       	exg d1, d1
   4:	ae 05       	exg d2, d1
   6:	ae 15       	exg d3, d1
   8:	ae 25       	exg d4, d1
   a:	ae 35       	exg d5, d1
   c:	ae 65       	exg d6, d1
   e:	ae 75       	exg d7, d1
  10:	ae 85       	exg x, d1
  12:	ae 95       	exg y, d1
  14:	ae c5       	exg cch, d1
  16:	ae d5       	exg ccl, d1
  18:	ae e5       	exg ccw, d1
  1a:	ae 68       	exg d6, x
  1c:	ae 79       	exg d7, y
  1e:	ae 8e       	exg x, ccw
  20:	ae 97       	sex y, d7
  22:	ae cd       	exg cch, ccl
