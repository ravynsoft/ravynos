# 1 "m.c"
! mmixal:= 8H LOC Data_Section
	.text ! mmixal:= 9H LOC 8B
	.p2align 2
	LOC @+(4-@)&3
	.global main
main	IS @
	GET $0,rJ
	PUSHJ $1,bar
	PUSHJ $1,bar
	PUT rJ,$0
	POP 1,0

	.data ! mmixal:= 8H LOC 9B
