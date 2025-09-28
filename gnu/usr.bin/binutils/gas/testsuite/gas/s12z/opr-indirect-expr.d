#objdump: -dr
#name:    Expressions in OPR indirect mode
#source:  opr-indirect-expr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	a7 fe 00 00 	ld d7, \[1\]
   4:	01 
			2: R_S12Z_OPR	FOO
