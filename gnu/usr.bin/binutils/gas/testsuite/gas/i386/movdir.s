# Check MOVDIR[I,64B] 32-bit instructions

	.allow_index_reg
	.text
_start:
	movdiri %eax, (%ecx)
	movdir64b (%ecx),%eax
	movdir64b (%si),%ax
	movdir64b foo, %cx
	movdir64b 0x1234, %cx

	.intel_syntax noprefix
	movdiri [ecx], eax
	movdiri dword ptr [ecx], eax
	movdir64b eax,[ecx]
	movdir64b ax,[si]
	movdir64b cx,ds:foo
	movdir64b cx,ds:0x1234
