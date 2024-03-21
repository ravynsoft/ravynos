#objdump: -d
#name:    
#source:  cmp-opr-rdirect.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	f0 ad       	cmp d2, \(d1,s\)
   2:	f1 88       	cmp d3, \(d2,x\)
   4:	f2 89       	cmp d4, \(d3,x\)
   6:	f3 99       	cmp d5, \(d3,y\)
   8:	f4 8a       	cmp d0, \(d4,x\)
   a:	f5 8b       	cmp d1, \(d5,x\)
   c:	f6 8e       	cmp d6, \(d6,x\)
   e:	f7 8f       	cmp d7, \(d7,x\)
