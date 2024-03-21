#objdump: -d
#name:    
#source:  cmp-opr-rindirect.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	f0 dd       	cmp d2, \[d1,y\]
   2:	f1 dd       	cmp d3, \[d1,y\]
   4:	f2 dc       	cmp d4, \[d0,y\]
   6:	f3 dc       	cmp d5, \[d0,y\]
   8:	f4 ce       	cmp d0, \[d6,x\]
   a:	f5 ce       	cmp d1, \[d6,x\]
   c:	f6 ce       	cmp d6, \[d6,x\]
   e:	f7 cf       	cmp d7, \[d7,x\]
