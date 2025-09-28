#objdump: -d -r
#name:    register prefix
#source:  reg-prefix.s --mreg-prefix=%

tmpdir/reg-prefix.o:     file format elf32-s12z


Disassembly of section .text:

00000000 <d0-0x1>:
   0:	01          	nop

00000001 <d0>:
   1:	c5 bc       	st d1, d0
   3:	c5 fa 00 00 	st d1, d0
   7:	01 
			5: R_S12Z_OPR	.text
