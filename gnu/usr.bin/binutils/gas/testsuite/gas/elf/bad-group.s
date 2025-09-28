	.section	.text.startup,"ax",%progbits
	.globl main
main:
	.type	main, %function
.LFB0:
	.section	.text.unlikely,"ax",%progbits
.L5:
	.globl __gxx_personality_v0
	.section	.gcc_except_table,"a",%progbits
	.uleb128 .L5-.LFB0
	.section	.data.foo,"awG",%progbits,foo,comdat
