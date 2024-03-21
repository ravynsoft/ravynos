	.text
	.arch armv4t
	.global _start
_start:
.LPIC0:	
	bx lr
	.align	2
	.word	var(tlsgd) + (. - .LPIC0 - 8)

	.section	.tbss,"awT",%nobits
	.align	2
	.type	var, %object
	.size	var, 4
var:
	.space	4
