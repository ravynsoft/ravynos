	.text

	.type	test1,%function
	.type	test1,%object
test1:
	.byte	0x0

	.type	test2,%object
	.type	test2,%function
test2:
	.byte	0x0

	.type	test3,%object
	.type	test3,%notype
test3:
	.byte	0x0

	.type	test4,%function
	.type	test4,%notype
test4:
	.byte	0x0

	.type	test5,%tls_object
	.type	test5,%notype
test5:
	.byte	0x0

	.type	test6,%gnu_indirect_function
	.type	test6,%notype
test6:
	.byte	0x0

	.type	test7,%function
	.type	test7,%notype
	.type	test7,%object
test7:
	.byte	0x0

	.type	test8,%object
	.type	test8,%tls_object
	.type	test8,%object
test8:
	.byte	0x0

	.type	test9,%function
	.type	test9,%gnu_indirect_function
	.type	test9,%function
test9:
	.byte	0x0
