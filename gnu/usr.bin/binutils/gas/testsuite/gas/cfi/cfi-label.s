	.text
cfilabel:
	.cfi_startproc
	.cfi_remember_state
	nop
	.globl cfi1
	.cfi_label cfi1
	.cfi_escape 0, 0, 0
	nop
	.cfi_label cfi2
	.cfi_escape 0, 0
	nop
	.cfi_label .Lcfi3
	.cfi_escape 0
	.cfi_restore_state
	ret
	.cfi_endproc
	.type cfilabel, STT_FUNC
	.size cfilabel, . - cfilabel
