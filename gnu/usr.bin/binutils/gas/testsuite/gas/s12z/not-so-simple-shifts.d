#objdump: -d
#name:    
#source:  not-so-simple-shifts.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	14 7b e3 71 	lsl.l d0, \(\+x\), #3
   4:	13 2a 8e    	lsr.p d5, \(d6,x\), #2
   7:	17 f9 f4 2d 	asl.w d7, \[45,p\], #13
   b:	76 
   c:	12 b8 84 00 	asr.b d4, \(145,d0\), #3
  10:	91 71 
