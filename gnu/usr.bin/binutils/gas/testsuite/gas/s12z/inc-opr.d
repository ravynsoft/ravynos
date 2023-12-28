#objdump: -d
#name:    
#source:  inc-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	9c 88       	inc.b \(d2,x\)
   2:	9d fe 00 04 	inc.w \[1234\]
   6:	d2 
   7:	9f 88       	inc.l \(d2,x\)
