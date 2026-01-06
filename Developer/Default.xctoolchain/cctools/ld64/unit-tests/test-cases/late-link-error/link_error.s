

#if __ppc__ || __ppc64__
	; illegal absolute load
_foo:	lis r2,ha16(_strcmp)
#endif

#if __i386__
	// illegal absolute load
_foo:	movl  _strcmp, %eax
#endif


#if __x86_64__
	// illegal external load
_foo:	movl  _strcmp(%rip), %eax
#endif

#if __arm__
	; illegal absolute load
_foo:	ldr r2, _strcmp
#endif
