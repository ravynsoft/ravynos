	.text
	.globl _start
	.type _start, @function
_start:
	ret

	.globl xyzzy	/* This symbol should be exported */
	.type xyzzy, @function
xyzzy:
	ret

	.section ".xyzzy_ptr","aw",%progbits
	.dc.a xyzzy
