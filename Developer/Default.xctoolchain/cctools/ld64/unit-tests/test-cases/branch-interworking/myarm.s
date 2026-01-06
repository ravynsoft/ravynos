
    .text
	.align 4
#if __arm__
	.code 32
	.align 2
#endif 

	.globl _myarm
_myarm:
    nop
#if __arm__
	//bl		_mythumb
	b		_mythumb
#elif __i386__ || __x86_64__
	jmp		_mythumb
#endif


    .subsections_via_symbols
