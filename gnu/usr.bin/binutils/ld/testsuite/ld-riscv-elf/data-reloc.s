	.macro	DATA symbol
.ifdef __64_bit__
	.quad	\symbol
.else
	.word	\symbol
.endif
	.endm
.ifdef __addr__
	.globl  addr_globl
addr_globl:
	DATA	addr_globl
addr_local:
	DATA	addr_local
.endif
.ifdef __abs__
	.hidden abs_local
	DATA	abs
	DATA	abs_local
.endif
.ifdef __undef__
	DATA	undef
.endif
