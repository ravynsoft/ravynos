	.eabi_attribute	Tag_CPU_arch, 11	@ V6-M
	.thumb
	.type myfunc, function
	.global myfunc
	.section .foo, "xa"
myfunc:
	bx	lr
