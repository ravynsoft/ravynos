#objdump: -dt
#name:    
#source:  brclr-symbols.s


.*:     file format elf32-s12z

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000001 l       .text	00000000 foo



Disassembly of section .text:

00000000 <foo-0x1>:
   0:	01          	nop

00000001 <foo>:
   1:	01          	nop
   2:	01          	nop
   3:	02 0c ff fe 	brclr d0, #1, foo
   7:	03 81 bd ff 	brset d1, d2, foo
   b:	fa 
   c:	02 a0 e7 ff 	brclr.b \(x\+\), #2, foo
  10:	f5 
  11:	03 c1 84 00 	brset.b \(23,d0\), d0, foo
  15:	17 ff f0 
  18:	02 a0 03 86 	brclr.b 902, #2, foo
  1c:	ff e9 