#objdump: -d -r
#name:    check that certain symbol labels are correctly accepted.
#source:  labels.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	c4 fa 00 00 	st d0, 0
   4:	00 
			2: R_S12Z_OPR	c
   5:	c4 bd       	st d0, d1
   7:	c5 fa 00 00 	st d1, 0
   b:	00 
			9: R_S12Z_OPR	xavier
