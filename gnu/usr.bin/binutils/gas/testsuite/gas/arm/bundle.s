	.syntax unified
	.bundle_align_mode 4

# We use these macros to test each pattern at every offset from
# bundle alignment, i.e. [0,16) by 2 or 4.

.macro offset_insn insn_name, offset, size
	.p2align 4
\insn_name\()_offset_\offset\():
	.rept \offset / \size
	bkpt
	.endr
	\insn_name
.endm

.macro test_offsets_arm insn_name
	.arm
	offset_insn \insn_name, 0, 4
	offset_insn \insn_name, 4, 4
	offset_insn \insn_name, 8, 4
	offset_insn \insn_name, 12, 4
.endm

.macro test_offsets_thumb insn_name
	.thumb
	offset_insn \insn_name, 0, 2
	offset_insn \insn_name, 2, 2
	offset_insn \insn_name, 4, 2
	offset_insn \insn_name, 6, 2
	offset_insn \insn_name, 8, 2
	offset_insn \insn_name, 10, 2
	offset_insn \insn_name, 12, 2
	offset_insn \insn_name, 14, 2
.endm

.macro test_arm
	add r0, r1
.endm

.macro test_thumb_2
	adds r0, r1
.endm
.macro test_thumb_4
	adds r8, r9
.endm

test_offsets_arm test_arm
test_offsets_thumb test_thumb_2
test_offsets_thumb test_thumb_4

# There are many relaxation cases for Thumb instructions.
# But we use as representative the simple branch cases.

.macro test_thumb_b_2
	b 0f
	bkpt 1
0:	bkpt 2
.endm
.macro test_thumb_b_4
	b far_target
.endm

test_offsets_thumb test_thumb_b_2
test_offsets_thumb test_thumb_b_4

# This is to set up a branch target surely too far for a short branch.
pad_for_far_target:
	.rept 1025
	bkpt 1
	.endr
far_target:
	bkpt 2
.p2align 4
	bkpt
