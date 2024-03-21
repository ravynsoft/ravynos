# Source file used to test missing (and illegal) operands for pseudo-instructions.

foo:
# nios2_modify_arg
	cmpgti r2, r3,
	cmpgtui r2, r2
	cmplei r2, r3,
	cmpleui r2, r2
	cmpgti ,,
	cmplei ,
	cmpleui

# nios2_negate_arg
	subi Lorem ipsum dolor sit amet, consectetur adipiscing elit,
	subi r2, r2,
	subi r2, r2
	subi ,,
	subi ,
	subi

# nios2_swap_args
	bgt r0, r2,
	bgtu ,,
	ble , r0,
	bleu foo,,
	cmpgt r2, r3,
	cmpgtu r2,,
	cmple , r3,
	cmpleu ,,
	bgtu ,
	ble

# nios2_insert_arg
	movi  ,
	movhi r0,
	movui ,r2
	movia ,
	movi

# nios2_append_arg
	mov r0,
	mov ,
	mov


