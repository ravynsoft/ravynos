#objdump: -d
#name:    
#source:  complex-shifts.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	14 73 e3 e6 	lsl.l d0, \(\+x\), \[345,s\]
   4:	00 01 59 
   7:	13 32 8e fb 	lsr.p d5, \(d6,x\), \(-s\)
   b:	17 f1 f4 2d 	asl.w d7, \[45,p\], \(278,y\)
   f:	d2 00 01 16 
  13:	12 b0 84 00 	asr.b d4, \(145,d0\), \(d0,s\)
  17:	91 ac 
