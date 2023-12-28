#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 branch

# Test the branch instructions.
.*:     file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 21400026 	beq	r4,r5,00000004 <[^>]*>
[	]*0: R_NIOS2_PCREL16	text_label
0+0004 <[^>]*> 2140000e 	bge	r4,r5,00000008 <[^>]*>
[	]*4: R_NIOS2_PCREL16	text_label
0+0008 <[^>]*> 2140002e 	bgeu	r4,r5,0000000c <[^>]*>
[	]*8: R_NIOS2_PCREL16	text_label
0+000c <[^>]*> 21400016 	blt	r4,r5,00000010 <[^>]*>
[	]*c: R_NIOS2_PCREL16	text_label
0+0010 <[^>]*> 21400036 	bltu	r4,r5,00000014 <[^>]*>
[	]*10: R_NIOS2_PCREL16	text_label
0+0014 <[^>]*> 2140001e 	bne	r4,r5,00000018 <[^>]*>
[	]*14: R_NIOS2_PCREL16	text_label
0+0018 <[^>]*> 00000006 	br	0000001c <[^>]*>
[	]*18: R_NIOS2_PCREL16	external_label
