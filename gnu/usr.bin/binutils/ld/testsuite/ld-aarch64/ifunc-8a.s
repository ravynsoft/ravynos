	.text
	.type start,"function"
	.global start
start:
	.type _start,"function"
	.global _start
_start:
	.type __start,"function"
	.global __start
__start:
	.type __start,"function"
        adrp    x0, :got:foo
        ldr     x0, [x0, #:got_lo12:foo]
