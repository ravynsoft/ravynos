foo = 0x12345678
bar = 0x12345679
baz = 0x1234567a
qux = 0x1234567b

	.text
	.globl main
	.align 4
main:
	entry   sp, 32
	movi    a2, foo
	movi    a3, bar
	movi    a4, baz
	movi    a5, qux
	movi    a6, 0
	movi    a7, 2

	loop    a2, .Lloop_end
	j       1f

2:
	movi    a2, 10
	loop    a2, 3f
	addi    a6, a6, 1
3:
	.rep 100
	nop
	.endr

	movi    a2, 10
	loop    a2, 3f
	addi    a6, a6, 1
3:
	.rep 100
	nop
	.endr

	.align  4
	.literal_position
1:
	beqi    a6, 2, 2b
	movi    a2, foo
	movi    a3, bar
	movi    a4, baz
	movi    a5, qux

	beqi    a6, 1, 1f

	addi    a6, a6, 1
.Lloop_end:

	.rep    16
	ill
	.endr

1:
	movi    a2, 0
	retw
