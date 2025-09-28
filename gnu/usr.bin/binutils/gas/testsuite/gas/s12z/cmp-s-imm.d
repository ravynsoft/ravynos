#objdump: -d
#name:    
#source:  cmp-s-imm.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 04 00 00 	cmp s, #0
   4:	00 
   5:	1b 02 71    	cmp s, #1
   8:	1b 02 70    	cmp s, #-1
   b:	1b 04 00 00 	cmp s, #255
   f:	ff 
  10:	1b 04 ff ff 	cmp s, #-256
  14:	00 
  15:	1b 04 00 7f 	cmp s, #32767
  19:	ff 
  1a:	1b 04 ff 80 	cmp s, #-32768
  1e:	00 
  1f:	1b 04 07 ff 	cmp s, #524287
  23:	ff 
  24:	1b 04 f8 00 	cmp s, #-524288
  28:	00 
