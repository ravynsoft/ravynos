; Test %hi/%lo handling.

foo:
	moviu r4,%u(foo+0x10000)
	movil r4,%l(foo+0x10000)

	moviu r4,%u 0x12348765
	movil r4,%l 0x12348765

	moviu r4,%u .+8
	movil r4,%l .+4
