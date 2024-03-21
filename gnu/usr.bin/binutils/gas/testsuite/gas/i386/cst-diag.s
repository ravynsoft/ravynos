	.text
const:
	add	$0x101, %cl
	add	$0x10001, %cx
	add	$0x100000001, %ecx
	add	0x100000001, %ecx

	add	$0x100, %cl
	add	$0x10000, %cx
	add	$0x100000000, %ecx
	add	0x100000000, %ecx

	add	$-0x101, %cl
	add	$-0x10001, %cx
	add	$-0x100000001, %ecx
	add	-0x100000001, %ecx

	add	$-0x100, %cl
	add	$-0x10000, %cx
	add	$-0x100000000, %ecx

	add	$0xffffffffffffff00, %cl
	add	$0xffffffffffff0000, %cx
	add	$0xffffffff00000000, %ecx
	add	0xffffffff00000000, %ecx

	# The next two might as well not have a disagnostic issued, but if
	# there is one (as is the case now), then it should be independent
	# of BFD64.
	and	$~0xff, %cl
	and	$~0xffff, %cx
	and	$~0xffffffff, %ecx
	and	~0xffffffff, %ecx

	and	$0xff+2, %cl
	and	$0xffff+2, %cx
	and	$0xffffffff+2, %ecx
	and	0xffffffff+2, %ecx

	and	$0xff*2, %cl
	and	$0xffff*2, %cx
	and	$0xffffffff*2, %ecx
	and	0xffffffff*2, %ecx

	.data
	.byte 0x101
	.byte -0x100
	.byte 0xffffffffffffff00
#	.byte ~0xffffffffffffff00
	.byte ~0xff
	.byte 0xff+2
	.byte 0xff*2

	.p2align 4
	.word 0x10001
	.word -0x10000
	.word 0xffffffffffff0000
#	.word ~0xffffffffffff0000
	.word ~0xffff
	.word 0xffff+2
	.word 0xffff*2

	.p2align 4
	.long 0x100000001
	.long -0x100000000
	.long 0xffffffff00000000
#	.long ~0xffffffff00000000
	.long ~0xffffffff
#	.long 0xffffffff+2
#	.long 0xffffffff*2

	.p2align 4
	.quad 0x100000001
	.quad -0x100000000
	.quad 0xffffffff00000000
#	.quad ~0xffffffff00000000
	.quad ~0xffffffff
#	.quad 0xffffffff+2
#	.quad 0xffffffff*2
