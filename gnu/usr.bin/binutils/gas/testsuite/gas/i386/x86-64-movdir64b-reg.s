# Check error for MOVDIR64B 32-bit instructions

	.allow_index_reg
	.text
_start:
	movdir64b (%esi),%rax
	movdir64b (%eip),%rax
	movdir64b (%rsi),%eax
	movdir64b (%rip),%eax

	.intel_syntax noprefix
	movdir64b rax,[esi]
	movdir64b eax,[rsi]
