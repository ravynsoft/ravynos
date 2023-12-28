#objdump: -d
#name:    Tests for shift and rotate instructions
#source:  shift.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	14 11 bb    	lsr d0, d3, d5
   3:	15 9a 76    	asr d1, d4, #13
   6:	15 84       	asr d1, d0, #1
   8:	17 68 fb    	lsl.b d7, \(-s\), #2
   b:	17 69 fb    	lsl.w d7, \(-s\), #2
   e:	17 6b fb    	lsl.l d7, \(-s\), #2
  11:	17 2a fb    	lsr.p d7, \(-s\), #2
  14:	10 3d f3    	lsr.w \(\+y\), #2
  17:	10 3e 8e    	lsr.p \(d6,x\), #2
  1a:	10 f4 bf    	asl d7, #1
  1d:	10 bc bd    	asr d1, #2
  20:	16 de 78    	asl d6, d6, #17
  23:	16 d6 78    	asl d6, d6, #16
