	.pred.safe_across_calls p1-p5,p16-p63
	.global foo#
	.section	.sdata,"aw",@progbits
	.align 8
	.type	foo#, @object
	.size	foo#, 8
foo:
	data8	@fptr(hidden#)
	.text
	.align 16
	.global hidden#
	.hidden	hidden#
	.proc hidden#
hidden:
	.prologue
	.body
	.bbb
	nop 0
	nop 0
	br.ret.sptk.many b0
	.endp hidden#
