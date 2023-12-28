#as: -mcpu=metac21
#objdump: -dr
#name: labelarithmetic

.*: +file format .*

Disassembly of section \.text:

00000000 <lbl1>:
.*:	862c0420 	          ADD       TXL1START,CPC0,#0x10

00000004 <lbl2>:
.*:	80002004 	          MOV       A0StP,CPC0
.*:	820000a0 	          ADD       A0StP,A0StP,#0x14
.*:	a3018c00 	          MOV       TXL1END,A0StP

00000010 <loop_start>:
.*:	00180404 	          MOV       D0Ar2,D0Ar4
.*:	a0fffffe 	          NOP

00000018 <loop_end>:
.*:	01180204 	          MOV       D1Ar1,D1Ar5
