# 1 "m.c"
! mmixal:= 8H LOC Data_Section
	.text ! mmixal:= 9H LOC 8B
	.data ! mmixal:= 8H LOC 9B
	.p2align 2
	LOC @+(4-@)&3
foo	IS @
	TETRA	#2
	.text ! mmixal:= 9H LOC 8B
	.p2align 2
	LOC @+(4-@)&3
	.global main
main	IS @
	SUBU $254,$254,8
	STOU $253,$254,0
	ADDU $253,$254,8
	LDT $0,foo
	ADDU $0,$0,1
	SET $0,$0
	STTU $0,foo
	SETL $0,0
	LDO $253,$254,0
	ADDU $254,$254,8
	POP 1,0

	.data ! mmixal:= 8H LOC 9B
