#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

[0-9a-f]+ <.text>:
   0:	23c0 00ed           	add.hi	r3,r3,0x3
   4:	24c0 012e           	add.ls	r4,r4,0x4
   8:	25c0 016f           	add.pnz	r5,r5,0x5
   c:	26c0 01b2           	add.cctst	r6,r6,0x6
  10:	27c0 01f3           	add.cstst	r7,r7,0x7
