	.hidden foo
 .ifdef HPUX
foo	.comm	4
 .else
	.comm	foo,4,4
 .endif
