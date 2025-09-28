	.cpu archs

	.weak symb
	.global __start
	.text
__start:
	ld	r0,[pcl,@symb@gotpc]
