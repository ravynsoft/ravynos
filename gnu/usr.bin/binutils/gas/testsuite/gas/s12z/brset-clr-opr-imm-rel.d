#objdump: -d
#name:    
#source:  brset-clr-opr-imm-rel.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	03 c0 e4 2d 	brset.b \[45,s\], #4, \*\+3
   4:	03 
   5:	03 e3 e6 00 	brset.w \[345,s\], #14, \*\+32
   9:	01 59 20 
   c:	03 99 e2 00 	brset.l \(345,s\), #9, \*\+309
  10:	01 59 81 35 
  14:	02 c0 e6 00 	brclr.b \[345,s\], #4, \*\+3
  18:	01 59 03 
  1b:	02 f3 e6 00 	brclr.w \[345,s\], #15, \*\+3087
  1f:	01 59 8c 0f 
  23:	02 fb e6 00 	brclr.l \[345,s\], #31, \*\+3
  27:	01 59 03 
