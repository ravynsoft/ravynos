	.text
	.org 0

	;; SLI/SLL instruction test

;SLI
	sli a
	sli b
	sli c
	sli d
	sli e
	sli h
	sli l
	sli (hl)
	sli (ix+7)
	sli (iy-9)

;SLL is alias for SLI
	sll a
	sll b
	sll c
	sll d
	sll e
	sll h
	sll l
	sll (hl)
	sll (ix+7)
	sll (iy-9)
