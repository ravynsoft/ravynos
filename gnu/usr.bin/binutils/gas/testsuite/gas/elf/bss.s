	.macro	bss name

	.long	0
	.long	,

	.balign	32

	.ifnes "\name", "struct"
	.ascii	""
	.asciz	""
	.endif

	.fill	1, 1
	.org	.+1
	.skip	1
	.sleb128
	.sleb128 0
	.uleb128
	.uleb128 0

	.ifndef okay

	.long	1
	.long	.
	.long	x
	.float	0.0

	.balign	32, -1

	.ifnes "\name", "struct"
	.ascii	"0"
	.asciz	"0"
	.endif

	.fill	1, 1, -1
	.org	.+1, -1
	.skip	1, -1
	.sleb128 -1
	.uleb128 1

	.endif

endof_\name:
	.endm

	.bss
	bss	bss

	.section .bss.local, "aw"
	bss	bss_local

	.section .private, "aw", %nobits
	bss	private

	.struct
	bss	struct
