#as: -mcpu=archs
#objdump: -dp

.*: +file format .*arc.*
private flags = 0x\d06: -mcpu=ARCv2HS .*


Disassembly of section .text:

00000000 <.text>:
   0:	4af7                	sub_s	r15,r2,r15
