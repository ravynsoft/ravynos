	.text
	.align 4
	.globl	_test
_test:
#if __i386__
	movl	L_foo$non_lazy_ptr, %eax
	movl	L_bar$non_lazy_ptr, %eax
	movl	L_other$non_lazy_ptr, %eax
	ret
#endif
#if __arm__ || __ppc__
	.long	L_foo$non_lazy_ptr
	.long	L_bar$non_lazy_ptr
	.long	L_other$non_lazy_ptr
#endif



	.section	__IMPORT,__pointers,non_lazy_symbol_pointers
L_foo$non_lazy_ptr:
.indirect_symbol _foo
	.long	0
	
	.section	__DATA,__one,non_lazy_symbol_pointers
L_bar$non_lazy_ptr:
.indirect_symbol _bar
	.long	0
	
	.section	__DATA,__two,non_lazy_symbol_pointers
L_other$non_lazy_ptr:
.indirect_symbol _other
	.long	0


.subsections_via_symbols
