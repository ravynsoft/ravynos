#as: -mcpu=arc700 -mnps400
#objdump: -dr

.*: +file format .*arc.*

Disassembly of section .text:

[0-9a-f]+ <.*>:
   0:	3a2f 1300           	dctcp	r10,r12
   4:	3e29 13cd           	dcip	r13,r14,r15
   8:	382f 2442           	dcet	r16,r17
   c:	3b20 2512           	dcet	r18,r19,r20
