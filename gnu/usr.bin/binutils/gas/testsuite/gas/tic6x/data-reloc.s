.data
.globl a
.globl ext1
.globl ext2
.globl ext3
a:
	.word ext1
	.word ext1 + 4
	.short ext2
	.short ext2 - 2
	.byte ext3
	.byte ext3 + 1
