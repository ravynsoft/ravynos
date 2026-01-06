

	.mod_init_func
#if __LP64__
	.quad	0x100000010
#else
	.long	0x1010
#endif


