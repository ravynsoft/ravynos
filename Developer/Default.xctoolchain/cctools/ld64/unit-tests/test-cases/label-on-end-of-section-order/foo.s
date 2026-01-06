	.lcomm	_mybss ,4, 2

	.text
	.align 4
	.globl	_main
_main:
#if __x86_64__
	movl $0, _mybss(%rip)
#elif __i386__
	movl $0, _mybss
#elif __arm__
	.long	_mybss
#endif

	.section __DATA, _stuff
	.align 4
_start_stuff:
	.long 0x0
	.long 0x0
_end_stuff:


	.subsections_via_symbols
	