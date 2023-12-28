#objdump: -d
#name:    
#source:  cmp-opr-inc.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	f0 e3       	cmp d2, \(\+x\)
   2:	f1 f3       	cmp d3, \(\+y\)
   4:	f2 c3       	cmp d4, \(-x\)
   6:	f3 d3       	cmp d5, \(-y\)
   8:	f4 fb       	cmp d0, \(-s\)
   a:	f6 ff       	cmp d6, \(s\+\)
   c:	f8 d7       	cmp x, \(y-\)
   e:	f8 c7       	cmp x, \(x-\)
  10:	f9 f7       	cmp y, \(y\+\)
  12:	f9 e7       	cmp y, \(x\+\)
