#objdump: -d
#name:    
#source:  simple-shift.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	14 55 b8    	lsl d0, d1, d2
   3:	11 12 bb    	lsr d3, d4, d5
   6:	17 d4 bd    	asl d7, d0, d1
   9:	10 91 ba    	asr d2, d3, d4
   c:	16 d5 b8    	asl d6, d1, d2
   f:	15 5a 76    	lsl d1, d4, #13
  12:	13 46       	lsl d5, d6, #1
