#objdump: -d
#name:    
#source:  opr-idx3-reg.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <L1>:
   0:	a9 c6 11 22 	ld y, \[1122867,x\]
   4:	33 
   5:	df fe 11 77 	neg.l \[1144678\]
   9:	66 
   a:	0c 9a fa 11 	mov.b #-102, 1162188
   e:	bb cc 
