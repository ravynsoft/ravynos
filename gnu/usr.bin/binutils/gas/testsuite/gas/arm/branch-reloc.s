@ Check that non-local branches with and without mode switching
@ produce the right relocations with appropriate in-place addends.

	.syntax unified

	.text
	.arm
	.global arm_glob_sym1
	.global arm_glob_sym2
	.global thumb_glob_sym1
	.global thumb_glob_sym2
	nop
	.type arm_glob_sym1, %function
arm_glob_sym1:
	bl thumb_glob_sym1
	bl thumb_glob_sym2
	bl thumb_sym1
	bl arm_glob_sym1
	bl arm_glob_sym2
	bl arm_sym1
	blx thumb_glob_sym1
	blx thumb_glob_sym2
	blx thumb_sym1
	blx arm_glob_sym1
	blx arm_glob_sym2
	blx arm_sym1
	nop
	bx lr

	.type arm_sym1, %function
arm_sym1:
	nop
	bx lr

	.thumb
	.thumb_func
	.type thumb_sym1, %function
thumb_sym1:
	bx lr

	.type thumb_glob_sym1, %function
	.thumb_func
	.thumb
thumb_glob_sym1:
	bx lr

	.section foo,"ax"

@ Add some space to avoid confusing objdump output: as we are
@ producing a relocatable file, objdump may match an address to
@ the wrong symbol (as symbols in different sections may have the same
@ address in the object file).
	.space 0x100

	.type thumb_glob_sym2, %function
	.thumb_func
	.thumb
thumb_glob_sym2:
	bl arm_glob_sym1
	bl arm_glob_sym2
	bl arm_sym2
	bl thumb_glob_sym1
	bl thumb_glob_sym2
	bl thumb_sym2
	blx arm_glob_sym1
	blx arm_glob_sym2
	blx arm_sym2
	blx thumb_glob_sym1
	blx thumb_glob_sym2
	blx thumb_sym2
	nop
	bx lr

	.type thumb_sym2, %function
thumb_sym2:
	nop
	bx lr

	.arm
	.type arm_sym2, %function
arm_sym2:
	bx lr

	.global arm_glob_sym2
	.type arm_glob_sym2, %function
arm_glob_sym2:
	bx lr
