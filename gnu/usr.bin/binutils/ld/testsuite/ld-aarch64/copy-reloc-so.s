	.global global_a
	.type	global_a, %object
	.size	global_a, 4

	.global global_b
	.type	global_b, %object
	.size	global_b, 4

	.global global_c
	.type	global_c, %object
	.size	global_c, 4

	.global global_d
	.type	global_d, %object
	.size	global_d, 4

	.data
global_a:
	.word 0xcafedead
global_b:
	.word 0xcafecafe
global_c:
	.word 0xdeadcafe
global_d:
	.word 0xdeaddead
