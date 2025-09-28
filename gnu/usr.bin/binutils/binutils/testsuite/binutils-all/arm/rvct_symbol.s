	.text
foo:
__tagsym$$0:
  add r0, r1, r2

	.data
	.global	global_a
__tagsym$$used0:
global_a:
	.word 0xcafedead

	.global	__tagsym$$used1
__tagsym$$used1:
global_b:
	.word 0xcafecafe
