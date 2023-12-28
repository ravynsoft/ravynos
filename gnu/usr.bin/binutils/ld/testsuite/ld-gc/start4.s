	.text
	.global _start
_start:
	.dc.a foo_xx
 .ifdef UNDERSCORE
	.dc.a ___start_xx, ___stop_xx
 .else
	.dc.a __start_xx, __stop_xx
 .endif

	.section xx,"a",%progbits,unique,0
	.global foo_xx
foo_xx:
	.dc.a 1

	.section xx,"a",%progbits,unique,1
	.global bar_xx
bar_xx:
	.dc.a 3
