#as: -mcpu=arcem -mcode-density -mdpfp
#objdump: -dp -M dpfp

.*: +file format .*arc.*
private flags = 0x\d05: -mcpu=ARCv2EM .*


Disassembly of section .text:

00000000 <.text>:
   0:	4af7                	sub_s	r15,r2,r15
   2:	3211 00c1           	dsubh12	r1,r2,r3
