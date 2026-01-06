
    .text
	.align 4

#if __thumb__
	.thumb_func _bar
	.code 16
#endif 

	.globl _bar
_bar:
    nop
#if __arm__
    bl		_foo
	blx		_foo
//	b		_foo
#endif




    .subsections_via_symbols
    