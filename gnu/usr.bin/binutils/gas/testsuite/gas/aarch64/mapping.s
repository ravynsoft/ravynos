	.text
	.global mapping
mapping:
	nop
	bl mapping

	.global another_mapping
another_mapping:
	nop
	bl another_mapping
	
	.data
	.word 0x123456

	.section foo,"ax"
	nop
