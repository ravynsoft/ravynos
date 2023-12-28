	.text
quoted:
	mov	"x(y)", %eax
	mov	"x(y)"(%eax), %eax
	mov	%es:"x(y)", %eax
	mov	%es:"x(y)"(%eax), %eax

	mov	"x(y", %eax
	mov	"x)y", %eax
	mov	"x?y", %eax
	mov	"x{y", %eax
	mov	"x{z}", %eax

	call	*"x(y)"
	call	*%es:"x(y)"
	call	%es:*"x(y)"

	mov	$"%eax", %eax
	mov	"%eax", %eax

	.intel_syntax noprefix
	mov	eax, "ecx"
	mov	eax, "xmm0"
	mov	eax, "not"
	mov	eax, "and"
	mov	eax, offset "edx"
