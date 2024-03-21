#objdump: -d -r
#name:    MOV instructions involving immediate operands which are relocatable expressions
#source:  mov-imm-reloc.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	0e 00 00 03 	mov.p #3, \(0,s\)
   4:	60 
			1: R_S12Z_OPR	xxx
   5:	0d 00 02 60 	mov.w #2, \(0,s\)
			6: R_S12Z_OPR	xxx
   9:	0f 00 00 00 	mov.l #1, \(0,s\)
   d:	01 60 
			a: R_S12Z_EXT32	xxx

