	.bundle_align_mode 5

# We use these macros to test each pattern at every offset from
# bundle alignment, i.e. [0,31].

.macro offset_insn insn_name, offset
	.p2align 5
\insn_name\()_offset_\offset\():
	.if \offset
	.space \offset, 0xf4
	.endif
	\insn_name
.endm

.macro test_offsets insn_name
	offset_insn \insn_name, 0
	offset_insn \insn_name, 1
	offset_insn \insn_name, 2
	offset_insn \insn_name, 3
	offset_insn \insn_name, 4
	offset_insn \insn_name, 5
	offset_insn \insn_name, 6
	offset_insn \insn_name, 7
	offset_insn \insn_name, 8
	offset_insn \insn_name, 9
	offset_insn \insn_name, 10
	offset_insn \insn_name, 11
	offset_insn \insn_name, 12
	offset_insn \insn_name, 13
	offset_insn \insn_name, 14
	offset_insn \insn_name, 15
	offset_insn \insn_name, 16
	offset_insn \insn_name, 17
	offset_insn \insn_name, 18
	offset_insn \insn_name, 19
	offset_insn \insn_name, 20
	offset_insn \insn_name, 21
	offset_insn \insn_name, 22
	offset_insn \insn_name, 23
	offset_insn \insn_name, 24
	offset_insn \insn_name, 25
	offset_insn \insn_name, 26
	offset_insn \insn_name, 27
	offset_insn \insn_name, 28
	offset_insn \insn_name, 29
	offset_insn \insn_name, 30
	offset_insn \insn_name, 31
.endm

# These are vanilla (non-relaxed) instructions of each length.
.macro test_1
	clc
.endm
.macro test_2
	add %eax,%eax
.endm
.macro test_3
	and $3,%eax
.endm
.macro test_4
	lock andl $3,(%rax)
.endm
.macro test_5
	mov $0x11223344,%eax
.endm
.macro test_6
	movl %eax,0x11223344(%rsi)
.endm
.macro test_7
	movl $0x11223344,0x7f(%rsi)
.endm
.macro test_8
	lock addl $0x11223344,0x10(%rsi)
.endm
.macro test_9
	lock addl $0x11223344,%fs:0x10(%rsi)
.endm
.macro test_10
	movl $0x11223344,0x7ff(%rsi)
.endm
.macro test_11
	lock addl $0x11223344,0x7ff(%rsi)
.endm
.macro test_12
	lock addl $0x11223344,%fs:0x7ff(%rsi)
.endm
.macro test_13
	lock addl $0x11223344,%fs:0x7ff(%r11)
.endm

test_offsets test_1
test_offsets test_2
test_offsets test_3
test_offsets test_4
test_offsets test_5
test_offsets test_6
test_offsets test_7
test_offsets test_8
test_offsets test_9
test_offsets test_10
test_offsets test_11
test_offsets test_12
test_offsets test_13

# The only relaxation cases are the jump instructions.
# For each of the three flavors of jump (unconditional, conditional,
# and conditional with prediction), we test a case that can be relaxed
# to its shortest form, and one that must use the long form.
.macro jmp_2
	jmp jmp_2_\@
	movl $0xdeadbeef,%eax
jmp_2_\@\():
	movl $0xb00b,%eax
.endm
.macro jmp_5
	jmp jmp_5_\@
	.rept 128
	clc
	.endr
jmp_5_\@\():
	movl $0xb00b,%eax
.endm

.macro cjmp_2
	jz cjmp_2_\@
	movl $0xdeadbeef,%eax
cjmp_2_\@\():
	movl $0xb00b,%eax
.endm
.macro cjmp_6
	jz cjmp_6_\@
	.rept 128
	clc
	.endr
cjmp_6_\@\():
	movl $0xb00b,%eax
.endm

.macro pjmp_3
	jz,pt pjmp_3_\@
	movl $0xdeadbeef,%eax
pjmp_3_\@\():
	movl $0xb00b,%eax
.endm
.macro pjmp_7
	jz,pt pjmp_7_\@
	.rept 128
	clc
	.endr
pjmp_7_\@\():
	movl $0xb00b,%eax
.endm

test_offsets jmp_2
test_offsets cjmp_2
test_offsets pjmp_3
test_offsets jmp_5
test_offsets cjmp_6
test_offsets pjmp_7

.p2align 5
	hlt
