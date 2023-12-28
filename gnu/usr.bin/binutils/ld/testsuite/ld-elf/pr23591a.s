 .ifdef UNDERSCORE
	.hidden ___start___sancov_cntrs
 .else
	.hidden __start___sancov_cntrs
 .endif
	.text
	.globl	_start
	.type	_start, %function
_start:
 .ifdef UNDERSCORE
	.dc.a	___start___sancov_cntrs
 .else
	.dc.a	__start___sancov_cntrs
 .endif
