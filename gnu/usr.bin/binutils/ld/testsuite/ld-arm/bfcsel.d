
.*:     file format elf32-.*arm


Disassembly of section .text:

00001000 <_start>:
    1000:	f101 e7ff 	bfcsel	4, 1001000 <bar>, 6, eq
			1000: R_ARM_THM_BF12	bar
    1004:	4623      	mov	r3, r4
    1006:	4611      	mov	r1, r2
    1008:	d0ff      	beq.n	100a <_start\+0xa>
    100a:	4613      	mov	r3, r2

Disassembly of section .foo:

01001000 <bar>:
 1001000:	4770      	bx	lr
