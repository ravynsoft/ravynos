#; $ as -o test.o gas-cfi-test.s && gcc -nostdlib -o test test.o

	.file   "a.c"
	.text
	.align 2
	.global foo
	.type   foo, %function
foo:
	.cfi_startproc
	mov	ip, sp
	.cfi_def_cfa ip, 0
	stmfd	sp!, {r0, r1, r2, r3}
	.cfi_def_cfa sp, 16
	stmfd	sp!, {fp, ip, lr, pc}
	.cfi_adjust_cfa_offset 16
	.cfi_rel_offset r11, 0
	.cfi_rel_offset lr, 8
	sub	fp, ip, #20
	.cfi_def_cfa fp, 16
	nop

	# Test fix for PR 16694 - the use of VFP registers in .cfi_offset directives.
	.cfi_offset r1,  -16
	.cfi_offset s1,  -20
	.cfi_offset d11, -48
	
	ldmea	fp, {fp, sp, pc}
	.cfi_endproc
	.size   foo, .-foo
