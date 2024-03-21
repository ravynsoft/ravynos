foo:	.word foo(TARGET2) + 0x1234
	.word foo + 0xcdef0000(TARGET2)
	.word (foo + 0x76543210)(TARGET2)
