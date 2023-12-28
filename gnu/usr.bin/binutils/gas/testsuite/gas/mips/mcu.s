	.text
	.set	mcu
	.ent	foo
	.globl	foo
foo:
	iret

	aclr	0, 0
	aclr	0, ($0)
	aclr	0, 0($0)
	aclr	1, 0($0)
	aclr	2, 0($0)
	aclr	3, 0($0)
	aclr	4, 0($0)
	aclr	5, 0($0)
	aclr	6, 0($0)
	aclr	7, 0($0)
	aclr	7, 0($2)
	aclr	7, 0($31)
	aclr	7, 2047($31)
	aclr	7, -2048($31)
	aclr	7, 2048($31)
	aclr	7, -2049($31)
	aclr	7, 32767($31)
	aclr	7, -32768($31)
	aclr	7, 65535($4)
	aclr	7, 65536($4)
	aclr	7, 0xffff0000($4)
	aclr	7, 0xffff8000($4)
	aclr	7, 0xffff0001($4)
	aclr	7, 0xffff8001($4)
	aclr	7, 0xf0000000($4)
	aclr	7, 0xffffffff($4)
	aclr	7, 0x12345678($4)

	aclr	1, %lo(foo)($3)
	aset	1, %lo(foo)($3)

	aset	0, 0
	aset	0, ($0)
	aset	0, 0($0)
	aset	1, 0($0)
	aset	2, 0($0)
	aset	3, 0($0)
	aset	4, 0($0)
	aset	5, 0($0)
	aset	6, 0($0)
	aset	7, 0($0)
	aset	7, 0($2)
	aset	7, 0($31)
	aset	7, 2047($31)
	aset	7, -2048($31)
	aset	7, 2048($31)
	aset	7, -2049($31)
	aset	7, 32767($31)
	aset	7, -32768($31)
	aset	7, 65535($4)
	aset	7, 65536($4)
	aset	7, 0xffff0000($4)
	aset	7, 0xffff8000($4)
	aset	7, 0xffff0001($4)
	aset	7, 0xffff8001($4)
	aset	7, 0xf0000000($4)
	aset	7, 0xffffffff($4)
	aset	7, 0x12345678($4)
	.end	foo

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
