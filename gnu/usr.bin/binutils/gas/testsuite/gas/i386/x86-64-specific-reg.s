# 64bit insns with special register requirements

	.text
special:
.irp reg1, ax, cx, dx, bx, sp, bp, si, di
	lodsb	%ds:(%r\reg1)

	stosb	%es:(%r\reg1)

	scasb	%es:(%r\reg1)

	insb	%dx, %es:(%r\reg1)

	outsb	%ds:(%r\reg1), %dx

	xlatb	%ds:(%r\reg1)

	movsb	%ds:(%r\reg1), %es:(%rdi)
	movsb	%ds:(%rsi), %es:(%r\reg1)

	cmpsb	%es:(%r\reg1), %ds:(%rsi)
	cmpsb	%es:(%rdi), %ds:(%r\reg1)

	mwait	%r\reg1, %rcx
	mwait	%rax, %r\reg1

	monitor	%r\reg1, %rcx, %rdx
	monitor	%rax, %r\reg1, %rdx
	monitor	%rax, %rcx, %r\reg1

	vmload	%r\reg1

	vmrun	%r\reg1

	vmsave	%r\reg1

	invlpga	%r\reg1, %ecx
	invlpga	%rax, %e\reg1

	skinit	%e\reg1
.endr

.irp reg1, 8, 9, 10, 11, 12, 13, 14, 15
	lodsb	%ds:(%r\reg1)

	stosb	%es:(%r\reg1)

	scasb	%es:(%r\reg1)

	insb	%dx, %es:(%r\reg1)

	outsb	%ds:(%r\reg1), %dx

	xlatb	%ds:(%r\reg1)

	movsb	%ds:(%r\reg1), %es:(%rdi)
	movsb	%ds:(%rsi), %es:(%r\reg1)

	cmpsb	%es:(%r\reg1), %ds:(%rsi)
	cmpsb	%es:(%rdi), %ds:(%r\reg1)

	mwait	%r\reg1, %rcx
	mwait	%rax, %r\reg1

	monitor	%r\reg1, %rcx, %rdx
	monitor	%rax, %r\reg1, %rdx
	monitor	%rax, %rcx, %r\reg1

	vmload	%r\reg1

	vmrun	%r\reg1

	vmsave	%r\reg1

	invlpga	%r\reg1, %ecx
	invlpga	%rax, %r\reg1\(d)

	skinit	%r\reg1\(d)
.endr

.irp n, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
	blendvpd %xmm\n, %xmm\n, %xmm\n
	blendvps %xmm\n, %xmm\n, %xmm\n
	pblendvb %xmm\n, %xmm\n, %xmm\n
.endr
