# Check xsave/xrstor
	.text
_start:
	xrstor		(%ebx)
	xsave		(%ebx)
	xsaveopt	(%ebx)
	xgetbv
	xsetbv

	.intel_syntax noprefix
	xrstor		[ecx]
	xsave		[ecx]
	xsaveopt	[ecx]

	.att_syntax prefix
avx:
	.arch generic32
	.arch .avx
	xsave	(%eax)
	xrstor	(%eax)

lwp:
	.arch generic32
	.arch .lwp
	xsave	(%eax)
	xrstor	(%eax)

mpx:
	.arch generic32
	.arch .mpx
	xsave	(%eax)
	xrstor	(%eax)

pku:
	.arch generic32
	.arch .ospke
	xsave	(%eax)
	xrstor	(%eax)
