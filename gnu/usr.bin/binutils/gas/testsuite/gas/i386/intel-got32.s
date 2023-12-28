.text
.intel_syntax noprefix
_start:
	mov	edx, [x1@GOTOFF + x2]
	ret

.equ x2, 4
