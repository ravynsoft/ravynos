	.syntax unified
	.bundle_align_mode 4

# We use these macros to test each pattern at every offset from
# bundle alignment, i.e. [0,16) by 2 or 4.

	size_arm = 4
	size_thumb = 2

.macro offset_sequence which, size, offset
	.p2align 4
\which\()_sequence_\size\()_offset_\offset\():
	.rept \offset / size_\which
	bkpt
	.endr
	test_sequence \size
.endm

.macro test_offsets_arm size
	.arm
	offset_sequence arm, \size, 0
	offset_sequence arm, \size, 4
	offset_sequence arm, \size, 8
	offset_sequence arm, \size, 12
.endm

.macro test_offsets_thumb size
	.thumb
	offset_sequence thumb, \size, 0
	offset_sequence thumb, \size, 2
	offset_sequence thumb, \size, 4
	offset_sequence thumb, \size, 6
	offset_sequence thumb, \size, 8
	offset_sequence thumb, \size, 10
	offset_sequence thumb, \size, 12
	offset_sequence thumb, \size, 14
.endm

.macro test_sequence size
	.bundle_lock
	adds r0, r1
	.rept \size - 1
	subs r0, r1
	.endr
	.bundle_unlock
.endm

	test_offsets_arm 1
	test_offsets_arm 2
	test_offsets_arm 3
	test_offsets_arm 4

	test_offsets_thumb 1
	test_offsets_thumb 2
	test_offsets_thumb 3
	test_offsets_thumb 4
	test_offsets_thumb 5
	test_offsets_thumb 6
	test_offsets_thumb 7
	test_offsets_thumb 8

	.arm
.p2align 4
	bkpt
