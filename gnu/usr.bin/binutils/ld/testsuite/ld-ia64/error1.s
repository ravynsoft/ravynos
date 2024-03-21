	.explicit
	.pred.safe_across_calls p1-p5,p16-p63
	.text
	.align 16
	.global _start#
	.proc _start#
_start:
	.prologue 12, 32
	.mii
	.save ar.pfs, r33
	alloc r33 = ar.pfs, 0, 3, 0, 0
	.save rp, r32
	mov r32 = b0
	mov r34 = r1
	.body
	;;
	.bbb
	nop 0
	nop 0
	br.call.sptk.many b0 = foo#
	;;
	.mmi
	nop 0
	mov r1 = r34
	mov b0 = r32
	.mib
	nop 0
	mov ar.pfs = r33
	br.ret.sptk.many b0
	.endp _start#
