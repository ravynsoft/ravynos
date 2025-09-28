	.text

subtract:
	sub	1, 2, 3
	sub.	5, 4, 3
	subo	1, 2, 1
	subo.	0, 2, 1
	subc	3, 4, 5
	subc.	3, 4, 5
	subco	1, 2, 3
	subco.  5, 6, 7

	e_subi		4, 5, 0x30
	e_subic		3, 6, 0x2
	e_subic. 	7, 8, 0x10

	e_sub16i	1, 2, 0xf
	e_sub2i.	5, 0x1
	e_sub2is	10, 0x100
