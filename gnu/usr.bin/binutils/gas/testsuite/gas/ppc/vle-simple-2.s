	.text

target0:
	e_bdnz	target1
	e_bdnzl	target1
	e_bdz	target2

target1:
	e_bdzl	target0
	e_beq	target0
	e_beq	cr1, target8

target2:
	e_beql cr0, target1
	e_beql target6
	e_bf 4*cr0+gt, target3

target3:
	e_bfl cr0*4+un, target0
	e_bge cr1, target1
	e_bge target5

target4:
	e_bgel cr2, target3
	e_bgel target4
	e_bgt cr0, target0
	e_bgt target0
	e_bgtl cr2, target2
	e_bgtl target2
	e_ble cr3, target5
	e_ble target5

target5:
	e_blel cr0, target4
	e_blel target4
	e_blt cr1, target3
	e_blt target3
	e_bltl target0
	e_bltl cr1, target0

target6:
	e_bne target7
	e_bne cr1, target0
	e_bnel cr0, target5
	e_bnel target5
	e_bng target9
	e_bng cr1, target4

target7:
	e_bngl cr2, target6
	e_bngl target8
	e_bnl cr1, target5
	e_bnl target5
	e_bnll cr3, target3
	e_bnll target3
	e_bns target2
	e_bns cr0, target2

target8:
	e_bnsl cr2, target0
	e_bnsl target6
	e_bnu cr1, target1
	e_bnu target1
	e_bnul target7
	e_bnul cr0, target3
	e_bso cr1, target4
	e_bso target4

target9:
	e_bsol cr0, target8
	e_bsol target8
	e_bt gt+cr0*4, target7
	e_btl lt+4*cr0, target5
	e_bun cr1, target4
	e_bun target4
	e_bunl cr2, target0
	e_bunl target9

