	.text
	.org 0
;;; exchange instructions
	ex af,af'
	exx
	ex de,hl
	ex (sp),hl
	ex (sp),ix
	ex (sp),iy
	
;;; AF operations
	daa
	cpl
	neg
	ccf
	scf

;;; CPU control
	nop
	halt
	di
	ei
	im 0
	im 1
	im 2
	