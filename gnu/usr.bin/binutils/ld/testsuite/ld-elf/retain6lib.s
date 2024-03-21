	.section	.text.bar,"ax"
	.global	bar
	.type	bar, %function
bar:
	.word 0

	.section	.text.retain_from_lib,"axR"
	.global	retain_from_lib
	.type	retain_from_lib, %function
retain_from_lib:
	.word 0

	.section	.text.discard_from_lib,"ax"
	.global	discard_from_lib
	.type	discard_from_lib, %function
discard_from_lib:
	.word 0
