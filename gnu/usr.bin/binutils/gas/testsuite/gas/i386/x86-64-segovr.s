# 64bit segment overrides

	.text
segovr:
.irp seg, ds, ss
 .irp reg, ax, cx, dx, bx, sp, bp, si, di, 8, 9, 10, 11, 12, 13, 14, 15
	mov	%\seg:(%r\reg), %eax
 .endr
.endr
