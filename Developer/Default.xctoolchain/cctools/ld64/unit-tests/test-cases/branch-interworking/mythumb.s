
    .text
	.align 4
_junk:
	  nop
	  nop
	  nop
	  nop
#if __arm__
	.syntax unified
	.thumb_func _mythumb
	.code 16
#endif 

	.globl _mythumb
_mythumb:
    nop
#if __arm__
	//bl		_myarm
	b.w		_myarm
#elif __i386__ || __x86_64__
	jmp		_myarm
#endif


    .subsections_via_symbols
