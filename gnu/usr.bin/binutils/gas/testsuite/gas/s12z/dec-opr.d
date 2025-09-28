#objdump: -d
#name:    
#source:  dec-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	ac ca       	dec.b \[d4,x\]
   2:	ad fe 00 00 	dec.w \[134\]
   6:	86 
   7:	af ad       	dec.l \(d1,s\)
