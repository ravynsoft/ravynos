#objdump: -d
#name:    Handle truncated instruction gracefully.
#source:  truncated.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	01          	nop
   1:	14          	Address 0x0*2 is out of bounds.
!!invalid!!
