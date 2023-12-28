#objdump: -d
#name:    
#source:  cmp-opr-xys.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	f0 61       	cmp d2, \(1,s\)
   2:	f1 42       	cmp d3, \(2,x\)
   4:	f2 43       	cmp d4, \(3,x\)
   6:	f3 53       	cmp d5, \(3,y\)
   8:	f4 44       	cmp d0, \(4,x\)
   a:	f5 45       	cmp d1, \(5,x\)
   c:	f6 46       	cmp d6, \(6,x\)
   e:	f7 47       	cmp d7, \(7,x\)
