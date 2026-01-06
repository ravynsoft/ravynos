

    .text
	.align 4

#if __thumb__
	.thumb_func _foo
	.code 16
#endif 
	.globl _foo
_foo:
    nop
#if __arm__
    bl		_bar
	blx		_bar
//	b		_bar

	.align 4
_space1:

#if __thumb2__
    .space 16*1024*1024 -100
#elif __thumb__
    .space 4*1024*1024 -100
#else
    .space 16*1024*1024 -100
#endif

#endif // __arm__



    .subsections_via_symbols
    