	.text
	.global bar
bar:
	.byte 0
	.data
	.dc.a foo
	.ifdef	HPUX
foo	.comm	4
	.else
	.comm	foo, 4, 4
	.endif
