	.text
	.intel_syntax noprefix
	.global _start
_start:
	ret

apic_read:
	mov	eax, [edi*4+APIC_BASE]
	ret

apic_write:
	mov	[edi*4+APIC_BASE], esi
	ret
