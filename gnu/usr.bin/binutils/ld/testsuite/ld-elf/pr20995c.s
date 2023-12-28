	.data
	.type rw,%object
	.globl rw
rw:
	.dc.a 0
	.size rw, . - rw

	.section .data.rel.ro,"aw",%progbits
	.type ro,%object
	.globl ro
ro:
	.dc.a 0
	.size ro, . - ro
