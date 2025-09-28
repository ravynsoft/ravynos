#objdump: -d
#name:    
#source:  brset-clr-reg-reg-rel.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	03 e1 bb 2d 	brset d5, d6, \*\+45
   4:	02 d1 b8 6f 	brclr d2, d1, \*-17
   8:	03 91 be ff 	brset d6, d3, \*-90
   c:	a6 
   d:	02 f1 bc ff 	brclr d0, d7, \*-90
  11:	a6 
  12:	02 81 bd ff 	brclr d1, d2, \*-190
  16:	42 
