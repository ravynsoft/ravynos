#objdump: -d
#name:    
#source:  and-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	6c c4 17    	and d0, \[23,x\]
   3:	6d d2 ff 8a 	and d1, \(-30000,y\)
   7:	d0 
   8:	68 c7       	and d2, \(x-\)
   a:	69 f3       	and d3, \(\+y\)
   c:	6a bb       	and d4, d5
   e:	6b fa 23 ca 	and d5, 2345678
  12:	ce 
  13:	6e fe 01 e2 	and d6, \[123456\]
  17:	40 
  18:	6f ac       	and d7, \(d0,s\)
