	.file	"test_pei.c"
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
.LFB0 = .
	.cfi_startproc
	addi.d	$r3,$r3,-32
	.cfi_def_cfa_offset 32
	st.d	$r22,$r3,24
	.cfi_offset 22, -8
	addi.d	$r22,$r3,32
	.cfi_def_cfa 22, 0
	addi.w	$r12,$r0,1			# 0x1
	st.w	$r12,$r22,-20
	addi.w	$r12,$r0,2			# 0x2
	st.w	$r12,$r22,-24
	ld.w	$r13,$r22,-20
	ld.w	$r12,$r22,-24
	mul.w	$r12,$r13,$r12
	slli.w	$r12,$r12,0
	or	$r4,$r12,$r0
	ld.d	$r22,$r3,24
	.cfi_restore 22
	addi.d	$r3,$r3,32
	.cfi_def_cfa_register 3
	jr	$r1
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (GNU) 12.1.0"
	.section	.note.GNU-stack,"",@progbits
