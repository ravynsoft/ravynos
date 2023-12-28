.text
	.section	.preinit_array,"aw"
	.short 42

	.section	.init_array,"aw"
	.short 43

	.section	.fini_array,"aw"
	.short 44

.text
	.global	main
	.type	main, @function
main:
	MOV #__preinit_array_start, R8
	MOV #__init_array_start, R9
	MOV #__fini_array_start, R10
  CALL @R8
  CALL @R9
  CALL @R10
	RET
