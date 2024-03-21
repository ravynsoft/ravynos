	.text

target0:
	se_beq	target3

target1:
	se_bf gt, target4

target2:
	se_bge  target2

target3:
	se_bgt target1

target4:
	se_ble target3
	se_blt target6

target5:
	se_bne target1
	se_bng target6

target6:
	se_bnl target4
	se_bns target5

target8:
	se_bnu target2
	se_bso target8

target9:
	se_bt eq, target6
	se_bun target9

