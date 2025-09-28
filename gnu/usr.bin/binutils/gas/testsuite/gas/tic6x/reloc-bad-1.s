# Test expressions not representable by relocations.
# Just one test so the resolution-time error isn't suppressed by other
# errors.
.globl a
.globl b
.data
d:
	.word a-b
