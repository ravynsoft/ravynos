.text
.intel_syntax noprefix
_start:
	movabs	eax, [x1@GOTOFF + x2]
	push	[rip + x1@GOTPCREL + x2]
	ret

.equ x2, 8
