	.section .text.foo,"axG",%progbits,foo_group,comdat
	.global foo
foo:
	.word 0

	.section .data.foo,"awG",%progbits,foo_group,comdat
	.global bar
bar:
	.word 1

	.section .dropme,"G",%progbits,foo_group,comdat
	.word 2
