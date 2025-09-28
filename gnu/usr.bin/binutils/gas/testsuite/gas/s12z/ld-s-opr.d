#objdump: -d
#name:    
#source:  ld-s-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 00 df    	ld s, \[d7,y\]
   3:	1b 00 fe 00 	ld s, \[8134\]
   7:	1f c6 
   9:	1b 00 ae    	ld s, \(d6,s\)
   c:	1b 00 fb    	ld s, \(-s\)
