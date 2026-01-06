

	 .section __MY, __data
_start:
		.long 0
_end:

# _var is a pointer to the end of the __MY/__data section
	.data
_var:
#if __x86_64__
	.quad _end
#else
	.long _end
#endif

	.subsections_via_symbols