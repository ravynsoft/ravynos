	.text
	.align 4

        .global __start
__start:
	add r0, pcl, @baz@tlsgd
	add r0, pcl, @bar@tlsgd
