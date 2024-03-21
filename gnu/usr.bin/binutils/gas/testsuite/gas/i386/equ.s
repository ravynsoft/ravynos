 .text
_start:

 .att_syntax prefix
 .equ r, -1
 .equ s, -1
	movl	$r, %eax
	movl	(r), %eax
 .equ r, xtrn
	movl	$r, %eax
	movl	r, %eax
 .equ r, %ecx
 .equ s, %fs
	testl	r, r
	movl	s:(r,r,4), r
 .equ z, y
 .equ y, x
 .equ x, %st(1)
	fadd	x
	fmul	y
	fsub	z

 .if r <> %ecx
 .err
 .endif
 .if r == s
 .err
 .endif

 .intel_syntax noprefix
 .equ r, -2
 .equ s, -2
	mov	eax, r
	mov	eax, FLAT:[r]
 .equ r, xtrn
	mov	eax, offset r
	mov	eax, [r]
 .equ r, edx
 .equ s, gs
	test	r, r
	mov	r, s:[r+r*8]
	mov	r, s:[8*r+r]
	fadd	x
	fmul	y
	fsub	z
 .equ c, b
 .equ b, a
 .equ a, st(7)
	fadd	a
	fmul	b
	fsub	c
 .equ r, edx + 4
	mov	eax, [r]
	mov	eax, [r]

 .if s <> gs
 .err
 .endif
 .if s == x
 .err
 .endif

 .equ r, -3
 .equ s, -3
