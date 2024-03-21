	.tls_common foo,4,4
	.tls_common bar,4,4

	.text
	.align 4

        .global __start
__start:
	ld r14, [pcl, @foo@tlsie]
	ld r15, [pcl, @bar@tlsie]
