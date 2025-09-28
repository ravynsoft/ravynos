#objdump: -d
#name:    
#source:  mul-imm.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	4c ec 85    	muls.b d0, d1, #-123
   3:	4d ed cf c7 	muls.w d1, d1, #-12345
   7:	48 ef ff ed 	muls.l d2, d1, #-1234567
   b:	29 79 
   d:	4c 6c 7b    	mulu.b d0, d1, #123
  10:	4d 6d 30 39 	mulu.w d1, d1, #12345
  14:	48 6f 00 12 	mulu.l d2, d1, #1234567
  18:	d6 87 
