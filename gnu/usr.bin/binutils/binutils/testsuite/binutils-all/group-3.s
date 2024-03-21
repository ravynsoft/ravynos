	.section .text.foo3,"axG",%progbits,foo3,comdat
	.global foo3
foo3:
	.word 0
	.section .data.bar3,"awG",%progbits,foo3,comdat
	.global bar3
bar3:
	.word 0
