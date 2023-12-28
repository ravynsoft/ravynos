	.text
	.global _start
	.weak foo
_start:
	leal	foo@GOTOFF(%eax), %eax
