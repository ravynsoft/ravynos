#objdump: -dr --prefix-addresses
#name: nds32 ji-jr instructions
#as:

# Test ls instructions

.*:     file format .*

Disassembly of section .text:
0+0000 <[^>]*> j	00000000 <foo>
			0: R_NDS32_25_PCREL_RELA	.text
			0: R_NDS32_RELAX_ENTRY	\*ABS\*
0+0004 <[^>]*> jal	00000004 <foo\+0x4>
			4: R_NDS32_25_PCREL_RELA	.text
0+0008 <[^>]*> jr[ 	]+\$r0
0+000c <[^>]*> jral[ 	]+\$lp, \$r0
0+0010 <[^>]*> ret[ 	]+\$lp
