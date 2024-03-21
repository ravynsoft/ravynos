#objdump: -d
#name:    
#source:  sbc-imm.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 70 12 34 	sbc d2, #4660
   4:	1b 71 12 34 	sbc d3, #4660
   8:	1b 72 12 34 	sbc d4, #4660
   c:	1b 73 12 34 	sbc d5, #4660
  10:	1b 74 ef    	sbc d0, #-17
  13:	1b 75 34    	sbc d1, #52
  16:	1b 76 56 78 	sbc d6, #1450744508
  1a:	9a bc 
  1c:	1b 77 98 77 	sbc d7, #-1737025662
  20:	17 82 
