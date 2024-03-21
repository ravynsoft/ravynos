#objdump: -d
#name:    Test of disassembler behaviour with invalid bit manipulation instructions
#source:  bit-manip-invalid.s


.*:     file format elf32-s12z


Disassembly of section \.text:

00000000 <\.text>:
   0:	01          	nop
   1:	03 a5 10 04 	brset\.w 4100, d4, \*\+6
   5:	06 
   6:	01          	nop
   7:	01          	nop
   8:	03 65 12    	brset d1, #4, \*\+18
   b:	01          	nop
   c:	01          	nop
   d:	ec 44       	bclr d0, #0
   f:	ec 7c       	bclr d0, #7
  11:	ed 5d       	bset d1, #3
  13:	ed 7d       	bset d1, #7
