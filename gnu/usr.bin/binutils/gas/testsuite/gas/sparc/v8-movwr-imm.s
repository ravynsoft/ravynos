! Make 'mov' and 'wr' aliases operate as per V8 SPARC Architecture Manual
	.text
foo:
	! wr Aliases
	wr	%l0,%asr1
	wr	%l0,%y
	wr	%l0,%psr
	wr	%l0,%wim
	wr	%l0,%tbr
	wr	%g0,%asr1
	wr	%g0,%y
	wr	%g0,%psr
	wr	%g0,%wim
	wr	%g0,%tbr
	wr	0,%asr1
	wr	0,%y
	wr	0,%psr
	wr	0,%wim
	wr	0,%tbr
	wr	-1,%asr1
	wr	-1,%y
	wr	-1,%psr
	wr	-1,%wim
	wr	-1,%tbr
	! mov Aliases
	mov	%l0,%asr1
	mov	%l0,%y
	mov	%l0,%psr
	mov	%l0,%wim
	mov	%l0,%tbr
	mov	%g0,%asr1
	mov	%g0,%y
	mov	%g0,%psr
	mov	%g0,%wim
	mov	%g0,%tbr
	mov	0,%asr1
	mov	0,%y
	mov	0,%psr
	mov	0,%wim
	mov	0,%tbr
	mov	-1,%asr1
	mov	-1,%y
	mov	-1,%psr
	mov	-1,%wim
	mov	-1,%tbr
