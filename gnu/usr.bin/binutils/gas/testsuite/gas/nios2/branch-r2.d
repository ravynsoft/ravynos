#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 branch
#as: -march=r2
#source: branch.s

# Test the branch instructions.
.*:     file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 0000292a 	beq	r4,r5,00000004 <[^>]*>
[	]*0: R_NIOS2_PCREL16	text_label
0+0004 <[^>]*> 00002912 	bge	r4,r5,00000008 <[^>]*>
[	]*4: R_NIOS2_PCREL16	text_label
0+0008 <[^>]*> 00002932 	bgeu	r4,r5,0000000c <[^>]*>
[	]*8: R_NIOS2_PCREL16	text_label
0+000c <[^>]*> 0000291a 	blt	r4,r5,00000010 <[^>]*>
[	]*c: R_NIOS2_PCREL16	text_label
0+0010 <[^>]*> 0000293a 	bltu	r4,r5,00000014 <[^>]*>
[	]*10: R_NIOS2_PCREL16	text_label
0+0014 <[^>]*> 00002922 	bne	r4,r5,00000018 <[^>]*>
[	]*14: R_NIOS2_PCREL16	text_label
0+0018 <[^>]*> 00000002 	br	0000001c <[^>]*>
[	]*18: R_NIOS2_PCREL16	external_label
