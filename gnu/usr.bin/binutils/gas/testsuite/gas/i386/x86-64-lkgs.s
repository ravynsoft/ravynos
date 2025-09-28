# Check 64bit LKGS instructions

	.allow_index_reg
	.text
_start:
	lkgs	%r12	 #LKGS
	lkgs    %r12w    #LKGS
	lkgsw   %r12w    #LKGS
	lkgs	0x10000000(%rbp, %r14, 8)	 #LKGS
	lkgs	(%r9)	 #LKGS
	lkgs	254(%rcx)	 #LKGS Disp32(fe000000)
	lkgs	-256(%rdx)	 #LKGS Disp32(00ffffff)

.intel_syntax noprefix
	lkgs	r12	 #LKGS
	lkgs	r12w	 #LKGS
	lkgsw	r12w	 #LKGS
	lkgs	WORD PTR [rbp+r14*8+0x10000000]	 #LKGS
	lkgs	WORD PTR [r9]	 #LKGS
	lkgs	WORD PTR [rcx+254]	 #LKGS Disp32(fe000000)
	lkgs	WORD PTR [rdx-256]	 #LKGS Disp32(00ffffff)
