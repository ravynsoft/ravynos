#objdump: -d
#name:    
#source:  lea.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	0a 44       	lea s, \(4,x\)
   2:	06 c0 7b    	lea d6, \(123,x\)
   5:	07 d1 16    	lea d7, \(-234,y\)
   8:	08 c1 ea    	lea x, \(-22,x\)
   b:	09 d0 16    	lea y, \(22,y\)
   e:	0a e0 16    	lea s, \(22,s\)
  11:	09 c1 d3    	lea y, \(-45,x\)
