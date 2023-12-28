	.section .text.foo,"axG",%progbits,.text.foo,comdat
	.global foo2
foo2:
	.word 0
	.section .data.bar,"awG",%progbits,.text.foo,comdat
	.global bar2
bar2:
	.word 0
