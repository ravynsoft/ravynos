	.tls_common foo,4,4
	.tls_common bar,4,4
	.text
	.align 1
	.type	do_test, @function
do_test:
	add.d foo:TPOFFGOT,$r0
	add.w bar:TPOFFGOT16,$r1
	.size	do_test, .-do_test
