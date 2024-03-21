	.tls_common foo,4,4
	.tls_common bar,4,4
	.text
	.align 1
	.type	do_test, @function
do_test:
	move.d foo:IE,$r0
	add.d bar:IE,$r1
	.size	do_test, .-do_test
