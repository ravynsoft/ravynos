	.text
	.org	0
	;; R800 extra instructions

	in f,(c)

	mulub a,b
	mulub a,c
	mulub a,d
	mulub a,e

	muluw hl,bc
	muluw hl,sp
