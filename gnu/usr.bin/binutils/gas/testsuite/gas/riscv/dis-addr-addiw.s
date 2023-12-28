.set addr_rv64_addiw_0a,   0xfffffffffffffff8  # 0xffffffe0 + 0x18 (sext:32->64)
.set addr_rv64_c_addiw_0a, 0xfffffffffffffffc  # 0xfffffff0 + 0x0c (sext:32->64)
.set addr_rv64_addiw_0b,           0x00000004  # 0xffffffe8 + 0x1c
.set addr_rv64_c_addiw_0b,         0x00000008  # 0xfffffff6 + 0x12
.set addr_rv64_addiw_1a,           0x7ffffff8  # 0x7fffffe0 + 0x18
.set addr_rv64_c_addiw_1a,         0x7ffffffc  # 0x7ffffff0 + 0x0c
.set addr_rv64_addiw_1b,   0xffffffff80000004  # 0x7fffffe8 + 0x1c (sext:32->64)
.set addr_rv64_c_addiw_1b, 0xffffffff80000008  # 0x7ffffff6 + 0x12 (sext:32->64)

	.text
	.global	_start
_start:
	.option	push
	.option	arch, -c
	# _start + 0x00
	auipc	t0, 0
	addiw	t1, t0, 0x18
	# _start + 0x08
	auipc	t2, 0
	addiw	t3, t2, 0x1c

	.option	pop
	# _start + 0x10
	auipc	t4, 0
	c.addiw	t4, 0x0c
	# _start + 0x16
	auipc	t5, 0
	c.addiw	t5, 0x12
