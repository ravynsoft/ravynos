	.text
	.set noreorder

foo:
	dmfgc0   $3,$29
	dmfgc0   $11,$20,5
	dmtgc0   $23,$2
	dmtgc0   $7,$14,2

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
