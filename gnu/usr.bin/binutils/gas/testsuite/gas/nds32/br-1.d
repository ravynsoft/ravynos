#objdump: -dr --prefix-addresses
#name: nds32 branch 1 instructions
#as:

# Test br-1 instructions

.*:     file format .*

Disassembly of section .text:
0+0000 <[^>]*> beq	\$r0, \$r1, 00000000 <foo>
			0: R_NDS32_15_PCREL_RELA	.text
			0: R_NDS32_INSN16	\*ABS\*
			0: R_NDS32_RELAX_ENTRY	\*ABS\*
0+0004 <[^>]*> bne	\$r0, \$r1, 00000004 <foo\+0x4>
			4: R_NDS32_15_PCREL_RELA	.text
			4: R_NDS32_INSN16	\*ABS\*
