# Check error for MOVDIR64B 32-bit instructions

	.allow_index_reg
	.text
_start:
	movdir64b (%si),%eax
	movdir64b (%esi),%ax

	.intel_syntax noprefix
	movdir64b eax,[si]
	movdir64b ax,[esi]
