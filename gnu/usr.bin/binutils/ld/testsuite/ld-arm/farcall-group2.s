@ Test to ensure that ARM calls exceeding 32Mb generate stubs.

	.text
myfunc:
	bl bar3
	bl bar4
	bl bar5
