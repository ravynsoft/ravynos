	.section .preinit_array.01000,"aw",%preinit_array
	.dc.a 0

	.section .init_array.01000,"aw",%init_array
	.dc.a 0

	.section .fini_array.01000,"aw",%fini_array
	.dc.a 0

	.section .preinit_array.01000,"awR",%preinit_array
	.dc.a 0

	.section .init_array.01000,"awR",%init_array
	.dc.a 0

	.section .fini_array.01000,"awR",%fini_array
	.dc.a 0

	.text
	.globl _start
_start:
	.dc.a 0
