	.text
test:
	lldp $2, $3, $4  /* No macro expansion needed */
	lldp $2, $3, 0($4)
	lldp $2, $0, 0x1234($4)
	lldp $2, $3, %lo(sync_mem)($2)
	lldp $2, $3, 0xffffffff01234567($3)
	lldp $0, $0, sync_mem($4)

	scdp $2, $3, $4  /* No macro expansion needed */
	scdp $2, $3, 0($4)
	scdp $2, $0, 0x1234($4)
	scdp $2, $3, %lo(sync_mem)($2)
	scdp $2, $3, 0xffffffff01234567($3)
	scdp $0, $0, sync_mem($4)

	.space 8

	.data
sync_mem:
	.word
	.word
	.space 8
