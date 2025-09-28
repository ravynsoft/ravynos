	.section	.tbss,"awT",%nobits
a10:
	.zero   0x7fef
a7fff:
	.zero	0x1
a8000:
	.zero	0x7fff
affff:
	.zero	0x1
a10000:
	.zero	0x7ffeffff
a7fffffff:
	.zero	0x1
a80000000:
	.zero	0x7fff0000
affff0000:
	.zero	0x0000ffff
affffffff:
	.zero	0x1234
a100001233:
	.zero	0x123356787e78
a1234567890ab:
	.zero	0xa9866f55
a1234ffff0000:
	.zero	0xfffe
a1234fffffffe:
	.zero	0x6dcb00000003
a800000000001:
	.zero	0x7ffffffffffe
affffffffffff:
	.zero	0x1234

	.text
	movz	x0, #:tprel_g1:a10
	movk	x0, #:tprel_g0_nc:a10
	movz	x0, #:tprel_g1:a7fff
	movk	x0, #:tprel_g0_nc:a7fff
	movz	x0, #:tprel_g1:a8000
	movk	x0, #:tprel_g0_nc:a8000
	movz	x0, #:tprel_g1:affff
	movk	x0, #:tprel_g0_nc:affff
	movz	x0, #:tprel_g1:a10000
	movk	x0, #:tprel_g0_nc:a10000
	movz	x0, #:tprel_g1:a7fffffff
	movk	x0, #:tprel_g0_nc:a7fffffff
	movz	x0, #:tprel_g1:a80000000
	movk	x0, #:tprel_g0_nc:a80000000
	movz	x0, #:tprel_g1:affff0000
	movk	x0, #:tprel_g0_nc:affff0000
	movz	x0, #:tprel_g1:affffffff
	movk	x0, #:tprel_g0_nc:affffffff

	movz	x0, #:tprel_g2:a10
	movk	x0, #:tprel_g1_nc:a10
	movk	x0, #:tprel_g0_nc:a10
	movz	x0, #:tprel_g2:a7fff
	movk	x0, #:tprel_g1_nc:a7fff
	movk	x0, #:tprel_g0_nc:a7fff
	movz	x0, #:tprel_g2:a8000
	movk	x0, #:tprel_g1_nc:a8000
	movk	x0, #:tprel_g0_nc:a8000
	movz	x0, #:tprel_g2:affff
	movk	x0, #:tprel_g1_nc:affff
	movk	x0, #:tprel_g0_nc:affff
	movz	x0, #:tprel_g2:a10000
	movk	x0, #:tprel_g1_nc:a10000
	movk	x0, #:tprel_g0_nc:a10000
	movz	x0, #:tprel_g2:a7fffffff
	movk	x0, #:tprel_g1_nc:a7fffffff
	movk	x0, #:tprel_g0_nc:a7fffffff
	movz	x0, #:tprel_g2:a80000000
	movk	x0, #:tprel_g1_nc:a80000000
	movk	x0, #:tprel_g0_nc:a80000000
	movz	x0, #:tprel_g2:affff0000
	movk	x0, #:tprel_g1_nc:affff0000
	movk	x0, #:tprel_g0_nc:affff0000
	movz	x0, #:tprel_g2:affffffff
	movk	x0, #:tprel_g1_nc:affffffff
	movk	x0, #:tprel_g0_nc:affffffff
	movz	x0, #:tprel_g2:a100001233
	movk	x0, #:tprel_g1_nc:a100001233
	movk	x0, #:tprel_g0_nc:a100001233
	movz	x0, #:tprel_g2:a1234567890ab
	movk	x0, #:tprel_g1_nc:a1234567890ab
	movk	x0, #:tprel_g0_nc:a1234567890ab
	movz	x0, #:tprel_g2:a1234ffff0000
	movk	x0, #:tprel_g1_nc:a1234ffff0000
	movk	x0, #:tprel_g0_nc:a1234ffff0000
	movz	x0, #:tprel_g2:a1234fffffffe
	movk	x0, #:tprel_g1_nc:a1234fffffffe
	movk	x0, #:tprel_g0_nc:a1234fffffffe
	movz	x0, #:tprel_g2:a800000000001
	movk	x0, #:tprel_g1_nc:a800000000001
	movk	x0, #:tprel_g0_nc:a800000000001
	movz	x0, #:tprel_g2:affffffffffff
	movk	x0, #:tprel_g1_nc:affffffffffff
	movk	x0, #:tprel_g0_nc:affffffffffff
	ret
