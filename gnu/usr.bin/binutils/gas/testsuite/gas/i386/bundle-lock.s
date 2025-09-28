	.bundle_align_mode 5

# We use these macros to test each pattern at every offset from
# bundle alignment, i.e. [0,31].

.macro offset_sequence size, offset
	.p2align 5
sequence_\size\()_offset_\offset\():
	.if \offset
	.space \offset, 0xf4
	.endif
	test_sequence \size
.endm

.macro test_offsets size
	offset_sequence \size, 0
	offset_sequence \size, 1
	offset_sequence \size, 2
	offset_sequence \size, 3
	offset_sequence \size, 4
	offset_sequence \size, 5
	offset_sequence \size, 6
	offset_sequence \size, 7
	offset_sequence \size, 8
	offset_sequence \size, 9
	offset_sequence \size, 10
	offset_sequence \size, 11
	offset_sequence \size, 12
	offset_sequence \size, 13
	offset_sequence \size, 14
	offset_sequence \size, 15
	offset_sequence \size, 16
	offset_sequence \size, 17
	offset_sequence \size, 18
	offset_sequence \size, 19
	offset_sequence \size, 20
	offset_sequence \size, 21
	offset_sequence \size, 22
	offset_sequence \size, 23
	offset_sequence \size, 24
	offset_sequence \size, 25
	offset_sequence \size, 26
	offset_sequence \size, 27
	offset_sequence \size, 28
	offset_sequence \size, 29
	offset_sequence \size, 30
	offset_sequence \size, 31
.endm

.macro test_sequence size
	.bundle_lock
	clc
	.rept \size - 1
	cld
	.endr
	.bundle_unlock
.endm

	test_offsets 1
	test_offsets 2
	test_offsets 3
	test_offsets 4
	test_offsets 5
	test_offsets 6
	test_offsets 7
	test_offsets 8
	test_offsets 9
	test_offsets 10
	test_offsets 11
	test_offsets 12
	test_offsets 13
	test_offsets 14
	test_offsets 15
	test_offsets 16
	test_offsets 17
	test_offsets 18
	test_offsets 19
	test_offsets 20
	test_offsets 21
	test_offsets 22
	test_offsets 23
	test_offsets 24
	test_offsets 25
	test_offsets 26
	test_offsets 27
	test_offsets 28
	test_offsets 29
	test_offsets 30
	test_offsets 31
	test_offsets 32

.p2align 5
	# Nested .bundle_lock.
	.bundle_lock
	clc
	.bundle_lock
	cld
	.bundle_unlock
	clc
	.bundle_unlock

.p2align 5
	hlt
