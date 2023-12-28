# Check EVEX instructions

	.allow_index_reg
	.text
_start:
	.insn EVEX.LIG.F3.0F.W1 0x2a, %eax,{rd-sae},%xmm5,%xmm6
	.insn EVEX.LIG.F2.0F.W0 0x2a, %eax,{rd-sae},%xmm5,%xmm6
	.insn EVEX.LIG.F2.0F.W1 0x2a, %eax,{rd-sae},%xmm5,%xmm6
	.insn EVEX.LIG.F3.0F.W1 0x7b, %eax,%xmm5,%xmm6
	.insn EVEX.LIG.F2.0F.W0 0x7b, %eax,%xmm5,%xmm6
	.insn EVEX.LIG.F2.0F.W1 0x7b, %eax,%xmm5,%xmm6
	.insn EVEX.LIG.F3.0F.W1 0x7b, %eax,{rd-sae},%xmm5,%xmm6
	.insn EVEX.LIG.F2.0F.W0 0x7b, %eax,{rd-sae},%xmm5,%xmm6
	.insn EVEX.LIG.F2.0F.W1 0x7b, %eax,{rd-sae},%xmm5,%xmm6
	.byte 0x62, 0xe1, 0x7e, 0x08, 0x2d, 0xc0
	.byte 0x62, 0xe1, 0x7c, 0x08, 0xc2, 0xc0, 0x00
