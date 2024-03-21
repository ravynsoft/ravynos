	.type	fd,@function
	.globl	fd

	.type	fh,@function
	.globl	fh
	.hidden fh

	.type	fp,@function
	.globl	fp
	.protected fp

	.type	fi,@function
	.globl	fi
	.internal fi

	.text
	.globl	_start
_start:
	nop

	.section ".text.f"
	.align	4
fd:
	nop
	.align	4
fh:
	nop
	.align	4
fp:
	nop
	.align	4
fi:
	nop
