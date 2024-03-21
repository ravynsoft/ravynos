#objdump: -d
#name:    
#source:  mul-opr-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	4c c2 ff fb 	muls.bb d0, \(s\+\), \(-s\)
   4:	4d 46 e0 2d 	mulu.bw d1, \(45,s\), \(d0,s\)
   8:	ac 
   9:	48 ca e5 d3 	muls.bp d2, \[-45,s\], \[1239\]
   d:	fe 00 04 d7 
  11:	49 4e c5 6f 	mulu.bl d3, \[-145,x\], \(\+x\)
  15:	e3 
  16:	4c d2 e7 d3 	muls.wb d0, \(x\+\), \(-y\)
