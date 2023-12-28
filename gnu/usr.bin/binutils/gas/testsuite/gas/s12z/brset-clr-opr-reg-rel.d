#objdump: -d
#name:    
#source:  brset-clr-opr-reg-rel.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <L1-0x6>:
   0:	02 85 d0 22 	brclr.w \(34,y\), d2, \*\+3034
   4:	8b da 

00000006 <L1>:
   6:	03 c1 d3 81 	brset.b \(-y\), d0, \*\+434
   a:	b2 
   b:	03 9d e3 88 	brset.l \(\+x\), d3, \*\+2134
   f:	56 
  10:	02 ad c4 22 	brclr.l \[34,x\], d4, L1
  14:	ff f6 
