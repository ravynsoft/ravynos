	.text
	.set noreorder

foo:
	mfgc0   $3,$29
	mfgc0   $11,$20,5
	mtgc0   $23,$2
	mtgc0   $7,$14,2

	hypcall
	hypcall 0x256

	tlbginv
	tlbginvf
	tlbgp
	tlbgr
	tlbgwi
	tlbgwr

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
