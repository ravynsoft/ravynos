
#if __arm__

  .section __MY,__text,regular,pure_instructions
	.align 4

#if __thumb__
	.thumb_func _bar
	.code 16
#endif 
	.globl _bar
_bar:
    nop
    bl		_foo
    blx		_foo2
    bl		_myweak1


#endif // __arm__



    .subsections_via_symbols
    