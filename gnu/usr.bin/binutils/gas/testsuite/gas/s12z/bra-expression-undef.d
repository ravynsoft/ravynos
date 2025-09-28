#objdump: -dr
#name:    pc_relative expressions without a definition
#source:  bra-expression-undef.s



.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	01          	nop
   1:	01          	nop
   2:	20 80 0b    	bra \*\+11
			3: R_S12Z_PCREL_7_15	loop\+0x8000
   5:	01          	nop
   6:	02 c0 bc 80 	brclr.b d0, #4, \*\+23
   a:	17 
			9: R_S12Z_PCREL_7_15	loop\+0x18000
   b:	01          	nop
   c:	0b 06 80 20 	tbne d6, \*\+32
			e: R_S12Z_PCREL_7_15	loop\+0x10000
  10:	01          	nop
