#objdump: -d
#name:    
#source:  lea-immu18.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	18 55       	lea x, \(85,x\)
   2:	19 aa       	lea y, \(-86,y\)
   4:	1a 12       	lea s, \(18,s\)
