#objdump: -d
#name:    
#source:  ld-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	a4 dc       	ld d0, \[d0,y\]
   2:	a5 cd       	ld d1, \[d1,x\]
   4:	a0 cd       	ld d2, \[d1,x\]
   6:	a1 80 04 d2 	ld d3, \(1234,d2\)
   a:	a2 e2 ff fb 	ld d4, \(-1234,s\)
   e:	2e 
   f:	a3 bf       	ld d5, d7
  11:	a6 88       	ld d6, \(d2,x\)
  13:	a7 d9       	ld d7, \[d3,y\]
  15:	a8 ac       	ld x, \(d0,s\)
  17:	a9 cd       	ld y, \[d1,x\]
