	.file "big-obj.s"

	.irp n,0,1,2,3,4
	.irp m,0,1,2,3,4,5,6,7,8,9
	.irp c,0,1,2,3,4,5,6,7,8,9
	.irp d,0,1,2,3,4,5,6,7,8,9
	.irp u,0,1,2,3,4,5,6,7,8,9
	.globl a\n\m\c\d\u
	.section .data$a\n\m\c\d\u,"w"
a\n\m\c\d\u :
	.byte 1
	.endr
	.endr
	.endr
	.endr
	.endr
