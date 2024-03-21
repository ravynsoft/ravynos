# Check MOVDIR[I,64B] 64-bit instructions

	.allow_index_reg
	.text
_start:
	movdiri %rax, (%rcx)
	movdir64b (%rcx),%rax
	movdir64b (%ecx),%eax
	movdir64b foo(%rip),%rcx
	movdir64b foo(%eip),%ecx
	movdir64b foo, %ecx
	movdir64b 0x12345678, %ecx

	.intel_syntax noprefix
	movdiri [rcx],eax
	movdiri [rcx],rax
	movdiri dword ptr [rcx],eax
	movdiri qword ptr [rcx],rax
	movdir64b rax,[rcx]
	movdir64b eax,[ecx]
	movdir64b rcx,[rip+foo]
	movdir64b ecx,[eip+foo]
	movdir64b ecx,ds:foo
	movdir64b ecx,ds:0x12345678
