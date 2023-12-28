#objdump: -dr --prefix-addresses
#name: nds32 branch 2 instructions
#as:

# Test br-2 instructions

.*:     file format .*

Disassembly of section .text:
0+0000 <[^>]*> beqz	\$r0, 00000000 <foo>
			0: R_NDS32_17_PCREL_RELA	.text
			0: R_NDS32_INSN16	\*ABS\*
			0: R_NDS32_RELAX_ENTRY	\*ABS\*
0+0004 <[^>]*> bgez	\$r0, 00000004 <foo\+0x4>
			4: R_NDS32_17_PCREL_RELA	.text
0+0008 <[^>]*> bgezal	\$r0, 00000008 <foo\+0x8>
			8: R_NDS32_17_PCREL_RELA	.text
0+000c <[^>]*> bgtz	\$r0, 0000000c <foo\+0xc>
			c: R_NDS32_17_PCREL_RELA	.text
0+0010 <[^>]*> blez	\$r0, 00000010 <foo\+0x10>
			10: R_NDS32_17_PCREL_RELA	.text
0+0014 <[^>]*> bltz	\$r0, 00000014 <foo\+0x14>
			14: R_NDS32_17_PCREL_RELA	.text
0+0018 <[^>]*> bltzal	\$r0, 00000018 <foo\+0x18>
			18: R_NDS32_17_PCREL_RELA	.text
