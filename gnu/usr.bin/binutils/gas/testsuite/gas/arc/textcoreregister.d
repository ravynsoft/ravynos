#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

[0-9a-f]+ <.text>:
   0:	2000 00b8           	add	accX,r0,r2
   4:	2100 00f9           	add	accY,r1,r3
   8:	2000 70c2           	add	r2,accX,r3
   c:	2100 7e38           	add	accX,accY,accX
