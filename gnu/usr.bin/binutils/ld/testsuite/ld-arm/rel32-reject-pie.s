	.text
	.global main
main:
	b main

	.data
rel:
	.word non_global - .
