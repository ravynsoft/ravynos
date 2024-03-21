	.text
	.balign 4096
vtext:
	.p2align 4,,15
	.globl	_start
	.type	_start, @function
_start:
	ret
	.size	_start, .-_start
	.globl	vdata
	.data
	.align 4096
	.type	vdata, @object
	.size	vdata, 4
vdata:
	.long	5
	.comm	vbss,65536,4096
	.align 16
	.type	local, @object
	.size	local, 24
local:
	.byte	77
	.zero	7
	.dc.a	local
	.dc.a	0
