	.syntax unified
	.text
	.thumb
	.global foo
foo:
	.irp m, add.w, sub.w, addw, subw
	   \m sp, r7, #1
	.endr

	.irp m, bic, sbcs, and, eor
	   \m r7, sp, r2
	.endr

	.irp m, smlabb, smlatb, smlabt, smlatt
	   \m sp, sp, sp, sp
	   \m r0, sp, r3, r11
	.endr
