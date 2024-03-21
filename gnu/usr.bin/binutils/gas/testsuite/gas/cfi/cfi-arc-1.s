	.file	"foo.c"
	.section	.text
	.align	4
	.cfi_startproc
foo:
	st.a	r13,[sp,-48]
	.cfi_def_cfa_offset 48
	.cfi_offset r13, -48
.LCFI0:
	st	r14,[sp,4]
	st	r15,[sp,8]
	.cfi_adjust_cfa_offset 4
	.cfi_offset r14, -44
	.cfi_rel_offset r15, 12
	st.a	fp,[sp,-4]
	.cfi_rel_offset fp, 0
	mov	fp,sp
	.cfi_def_cfa_register fp
	j_s	[blink]
	.cfi_endproc
	.size	foo, .-foo
