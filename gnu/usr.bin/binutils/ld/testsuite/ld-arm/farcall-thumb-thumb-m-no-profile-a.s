	.eabi_attribute	Tag_CPU_arch, 11	@ V6-M
	.thumb
	.type _start, function
	.global _start
	.text
_start:
	bl	myfunc
	b	.
