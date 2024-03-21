#objdump: -d
#name:    
#source:  add-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	64 c5 21    	add d0, \[-223,x\]
   3:	65 e2 00 75 	add d1, \(30000,s\)
   7:	30 
   8:	60 c7       	add d2, \(x-\)
   a:	61 f3       	add d3, \(\+y\)
   c:	62 bf       	add d4, d7
   e:	63 17 be    	add d5, 6078
  11:	66 fe 01 1d 	add d6, \[73056\]
  15:	60 
  16:	67 8a       	add d7, \(d4,x\)
