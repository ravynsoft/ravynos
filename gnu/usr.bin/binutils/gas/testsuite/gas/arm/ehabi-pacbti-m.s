        .syntax unified
        .thumb
        .thumb_func
	.fnstart
	.cfi_startproc
	.pacspval
	pac	ip, lr, sp
	.cfi_register 143, 12
	push	{r0, r1, r2, r3}
	.save {r0, r1, r2, r3}
	.cfi_def_cfa_offset 16
	.cfi_offset 0, -16
	.cfi_offset 1, -12
	.cfi_offset 2, -8
	.cfi_offset 3, -4
	push	{r3, r7, ip, lr}
	.save {r3, r7, ra_auth_code, lr}
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	.cfi_offset 7, -28
	.cfi_offset 143, -24
	.cfi_offset 14, -20
	pop	{r3, r7, ip, lr}
	.cfi_restore 14
	.cfi_restore 143
	.cfi_restore 7
	.cfi_restore 3
	.cfi_def_cfa_offset 0
	add	sp, sp, #16
	.cfi_restore 3
	.cfi_restore 2
	.cfi_restore 1
	.cfi_restore 0
	.cfi_def_cfa_offset -16
	aut	ip, lr, sp
	bx	lr
	.cfi_endproc
	.fnend
