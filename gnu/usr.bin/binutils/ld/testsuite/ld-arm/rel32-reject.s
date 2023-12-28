	.text
	.global main
main:
	b main

	.data
	.extern ext_global
rel:
	.word ext_global - .
