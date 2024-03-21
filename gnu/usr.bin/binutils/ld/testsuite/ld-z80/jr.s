	.text
;	.org	0

	.globl	label1
	.globl	label2
	.globl	label3
	.globl	label4
	.globl	label5
	.globl	label6

	djnz	label1

	jr	label2
	jr	nz,label3
	jr	z,label4
	jr	nc,label5
	jr	c,label6

	djnz	.
	jr	.
	jr	nz,.
	jr	z,.
	jr	nc,.
	jr	c,.

.Ll1:
	djnz	.Ll1
.Ll2:
	jr	.Ll2
.Ll3:
	jr	nz,.Ll3
.Ll4:
	jr	z,.Ll4
.Ll5:
	jr	nc,.Ll5
.Ll6:
	jr	c,.Ll6

	djnz	.Lf1
	jr	.Lf2
	jr	nz,.Lf3
	jr	z,.Lf4
	jr	nc,.Lf5
	jr	c,.Lf6

.Lf1:
	ret
.Lf2:
	ret
.Lf3:
	ret
.Lf4:
	ret
.Lf5:
	ret
.Lf6:
	ret
	.end
