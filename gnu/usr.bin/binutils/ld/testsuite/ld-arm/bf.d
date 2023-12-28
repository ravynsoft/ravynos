
.*:     file format elf32-.*arm


Disassembly of section .text:

00001000 <_start>:
    1000:	f0df e7ff 	bf	2, 1001000 <bar>
			1000: R_ARM_THM_BF16	bar

Disassembly of section .foo:

01001000 <bar>:
 1001000:	4770      	bx	lr
