	.text
	.global _start
_start:
	.dc.a foo
 .ifdef UNDERSCORE
	.dc.a ___start_xx, ___stop_xx
 .else
	.dc.a __start_xx, __stop_xx
 .endif

	.section .text,"axG",%progbits,foo_group
	.global foo
foo:
	.dc.a 0

	.section xx,"aG",%progbits,foo_group
	.global foo_xx
foo_xx:
	.dc.a 1

	.section .text,"axG",%progbits,bar_group
	.global bar
bar:
	.dc.a 2

	.section xx,"aG",%progbits,bar_group
	.global bar_xx
bar_xx:
	.dc.a 3
