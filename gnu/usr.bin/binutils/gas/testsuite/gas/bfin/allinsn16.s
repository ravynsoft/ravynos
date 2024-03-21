# need to do some nested macro stuff to workaround
# missing 'for' loop and nested limits

# iterate 0x40 times
.macro __dw b, i, e
	.if \i < \e
		.dw \b + \i
		__dw \b, (\i + 1), \e
	.endif
.endm

# iterate 0x20 times
.macro _dw b, i, e
	.if \i < \e
		__dw \b, \i, \i + 0x40
		_dw \b, \i + 0x40, \e
	.endif
.endm

# iterate 0x4 times
.macro dw b, i, e
	.if \i < \e
		_dw \b, \i, \i + 0x800
		dw \b, \i + 0x800, \e
	.endif
.endm

dw 0x0000 0 0x2000
dw 0x2000 0 0x2000
dw 0x4000 0 0x2000
dw 0x6000 0 0x2000
dw 0x8000 0 0x2000
dw 0xa000 0 0x2000
