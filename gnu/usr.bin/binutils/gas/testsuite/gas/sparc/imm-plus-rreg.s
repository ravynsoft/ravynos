! simm13 + regrs1 address using r<0..31> instead of [goli]<0..7>
	.text
foo:
	ld	[10+%r8], %r1
	ld	[10+%r18], %r2
	st	%r2, [10+%r8]
	st	%r1, [10+%r18]
