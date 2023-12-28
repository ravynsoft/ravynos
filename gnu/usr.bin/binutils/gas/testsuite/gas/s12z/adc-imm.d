#objdump: -d
#name:    
#source:  adc-imm.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 50 12 34 	adc d2, #4660
   4:	1b 51 12 34 	adc d3, #4660
   8:	1b 52 12 34 	adc d4, #4660
   c:	1b 53 12 34 	adc d5, #4660
  10:	1b 54 12    	adc d0, #18
  13:	1b 55 34    	adc d1, #52
  16:	1b 56 00 56 	adc d6, #5666970
  1a:	78 9a 
  1c:	1b 57 00 98 	adc d7, #9991764
  20:	76 54 
