

	.mod_term_func
#if __LP64__
	.quad	_malloc + 0x100000010
#else
	.long	_malloc + 0x1010
#endif


