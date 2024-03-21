#objdump: -d
#name:    
#source:  st-s-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 01 aa    	st s, \(d4,s\)
   3:	1b 01 fe 00 	st s, \[1340\]
   7:	05 3c 
   9:	1b 01 9b    	st s, \(d5,y\)
   c:	1b 01 d3    	st s, \(-y\)
