.text ; foo: addr16 fstcw %es:(%si)
 fstsw; fstsw %ax;
 addr16 fstsw %ax ;addr16 rep cmpsw %es:(%di),%ss:(%si)

 es fwait

	fwait
	movl $0,%gs:fpu_owner_task

	.byte 0x66
	.byte 0xf2
	.byte 0x0f
	.byte 0x38
	.byte 0x17

	.byte 0xf2
	.byte 0x66
	.byte 0x0f
	.byte 0x54

	.byte 0xf2
	.byte 0x0f
	.byte 0x54

# data16 movsd %xmm4,(%edx)
	.byte 0xf2
	movupd %xmm4, (%edx)

# data16 movsd %xmm4,(%bp,%si)
	.byte 0xf2
	movupd %xmm4, (%bp,%si)

# lock data16 movsd %xmm4,(%bp,%si)
	.byte 0xf2
	.byte 0x67
	lock
	movupd %xmm4, (%edx)

# data16 movss %xmm4,(%edx)
	.byte 0xf3
	movupd %xmm4, (%edx)

# lock data16 movss %xmm4,(%bp,%si)
	.byte 0xf3
	.byte 0x67
	lock
	movupd %xmm4, (%edx)

# repz data16 movsd %xmm4,(%bp,%si)
	repz
	.byte 0x67
	.byte 0xf2
	movupd %xmm4, (%edx)

# data16 movss %xmm4,%ds:(%edx)
	.byte 0xf3
	.byte 0x66
	ds
	movups %xmm4, (%edx)

# data16 movsd %xmm4,%ss:(%edx)
	.byte 0xf2
	.byte 0x66
	movups %xmm4, %ss:(%edx)

# repz lock data16 movsd %xmm4,%ss:(%edx)
	repz
	lock
	.byte 0xf2
	.byte 0x66
	movups %xmm4, %ss:(%edx)

# data16 ds movsd %xmm4,%ss:(%edx)
	.byte 0xf2
	.byte 0x66
	ds
	movups %xmm4, %ss:(%edx)

# data16 ds movsd %xmm4,%ss:(%bp,%si)
	.byte 0xf2
	.byte 0x67
	.byte 0x66
	ds
	movups %xmm4, %ss:(%edx)

# lock data16 ds movsd %xmm4,%ss:(%bp,%si)
	.byte 0xf2
	.byte 0x67
	lock
	.byte 0x66
	ds
	movups %xmm4, %ss:(%edx)

# data16 ds movss %xmm4,%ss:(%edx)
	.byte 0xf3
	.byte 0x66
	ds
	movups %xmm4, %ss:(%edx)

# lock data16 ds movss %xmm4,%ss:(%edx)
	.byte 0xf3
	lock
	.byte 0x66
	ds
	movups %xmm4, %ss:(%edx)

# repz data16 ds movsd %xmm4,%ss:(%bp,%si)
	.byte 0xf3
	.byte 0x67
	.byte 0xf2
	.byte 0x66
	ds
	movups %xmm4, %ss:(%edx)

# repnz; xchg %ax,%ax
	repnz
	xchg %ax, %ax

# repnz; addr16 xchg %ax,%ax
	repnz
	.byte 0x67
	xchg %ax, %ax

# repnz; addr16 lock xchg %ax,%ax
	repnz
	.byte 0x67
	lock
	xchg %ax, %ax

# data16 pause
	repz
	xchg %ax, %ax

# addr16 lock data16 pause
	.byte 0xf3
	.byte 0x67
	lock
	xchg %ax, %ax

# repz; addr16; repnz; xchg %ax,%ax
	repz
	.byte 0x67
	repnz
	xchg %ax, %ax

# repnz; ds nop
	repnz
	ds
	nop

# repnz; lock addr16 ds nop
	repnz
	lock
	.byte 0x67
	ds
	nop

# ds pause
	.byte 0xf3
	ds
	nop

# data16 ds pause
	.byte 0xf3
	.byte 0x66
	ds
	nop

# lock ds pause
	.byte 0xf3
	lock
	ds
	nop

# lock addr16 ds pause
	.byte 0xf3
	lock
	.byte 0x67
	ds
	nop

# repz; repnz; addr16 ds nop
	.byte 0xf3
	repnz
	.byte 0x67
	ds
	nop

# lock ss xchg %ax,%ax
	.byte 0x66
	lock
	ss
	nop

# repnz; ss nop
	repnz
	ss
	nop

# repnz; ss xchg %ax,%ax
	repnz
	.byte 0x66
	ss
	nop

# repnz; lock ss nop
	repnz
	lock
	ss
	nop

# repnz; lock addr16 ss nop
	repnz
	lock
	.byte 0x67
	ss
	nop

# ss pause
	repz
	ss
	nop

# addr16 ss pause
	repz
	.byte 0x67
	ss
	nop

# lock addr16 ss pause
	repz
	lock
	.byte 0x67
	ss
	nop

# repz; repnz; ss nop
	repz
	repnz
	ss
	nop

# repz; repnz; addr16 ss nop
	repz
	repnz
	.byte 0x67
	ss
	nop

# repz; lock; repnz; ss xchg %ax,%ax
	repz
	lock
	repnz
	.byte 0x66
	ss
	nop

# ds ss xchg %ax,%ax
	.byte 0x66
	ds
	ss
	nop

# addr16 ds ss xchg %ax,%ax
	.byte 0x67
	.byte 0x66
	ds
	ss
	nop

# addr16 lock ds ss xchg %ax,%ax
	.byte 0x67
	lock
	.byte 0x66
	ds
	ss
	nop

# data16 ds ss pause
	repz
	.byte 0x66
	ds
	ss
	nop

# lock data16 ds ss pause
	repz
	lock
	.byte 0x66
	ds
	ss
	nop

# repz; repnz; addr16 ds ss nop
	repz
	repnz
	.byte 0x67
	ds
	ss
	nop

# repz; addr16; repnz; ds ss xchg %ax,%ax
	repz
	.byte 0x67
	repnz
	.byte 0x66
	ds
	ss
	nop

# repz; rdseed %eax
	repz
	rdseed %eax

	nop

# repz; rdrand %eax
	repz
	rdrand %eax

	nop

# repnz; rdseed %eax
	repnz
	rdseed %eax

	nop

# repnz; rdrand %eax
	repnz
	rdrand %eax

	nop

	repz; movaps %xmm7, %xmm7
	int $3

# "repz" vmovaps %xmm7, %xmm7
	.insn VEX.128.f3.0f.W0 0x28, %xmm7, %xmm7

	int $3

# "repnz" {vex3} vmovaps %xmm7, %xmm7
	.insn {vex3} VEX.128.f2.0f.W0 0x28, %xmm7, %xmm7

	int $3

# "EVEX.W1" vmovaps %xmm7, %xmm7
	.insn EVEX.128.0f.W1 0x28, %xmm7, %xmm7

	int $3

# "repz" vmovaps %xmm7, %xmm7
	.insn EVEX.128.f3.0f.W0 0x28, %xmm7, %xmm7

	int $3

# "EVEX.W0" vmovapd %xmm7, %xmm7
	.insn EVEX.128.66.0f.W0 0x28, %xmm7, %xmm7

	int $3

# "repnz" vmovapd %xmm7, %xmm7
	.insn EVEX.128.f2.0f.W1 0x28, %xmm7, %xmm7

	int $3

	.byte 0x66; vmovaps %xmm0, %xmm0
	repz; {vex3} vmovaps %xmm0, %xmm0
	repnz; vmovaps %xmm0, %xmm0
	lock; {evex} vmovaps %xmm0, %xmm0

	vcvtpd2dqx 0x20(%eax),%xmm0
	vcvtpd2dq 0x20(%eax){1to2},%xmm0
	vcvtpd2dqx 0x20(%eax),%xmm0

# Get a good alignment.
 .p2align	4,0
