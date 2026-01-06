	.cstring
LC0:
	.ascii "bar\0"
LC1:
	.ascii "coal\0"
	.text
	

	.section __TEXT, __mystring, cstring_literals
LC4:
	.ascii "bar\0"
LC5:
	.ascii "mycoal\0"


	.data
#if __LP64__
	.quad	 LC0
	.quad	 LC1
	.quad	 LC4
	.quad	 LC5
#else
	.long	 LC0
	.long	 LC1
	.long	 LC4
	.long	 LC5
#endif

	.subsections_via_symbols


