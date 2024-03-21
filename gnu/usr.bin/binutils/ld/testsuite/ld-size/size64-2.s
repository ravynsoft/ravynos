# 64-bit size relocation against TLS symbol in shared object
	.globl	xxx
	.section	.tbss,"awT",%nobits
	.p2align 2
	.type	xxx, %object
	.size	xxx, 40
xxx:
	.zero	40
	.globl	yyy
	.section	.tdata,"awT",%progbits
	.p2align 2
	.type	yyy, %object
	.size	yyy, 40
yyy:
	.zero	40
	.data
	.p2align 2
	.quad	xxx@SIZE
	.quad	yyy@SIZE
	.quad	zzz@SIZE
	.quad	zzz@SIZE-30
	.quad	zzz@SIZE+30
