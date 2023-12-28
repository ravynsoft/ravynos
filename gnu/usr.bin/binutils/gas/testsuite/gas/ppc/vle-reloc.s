	.text
	se_b	sub1
	se_bl	sub1
	se_bc	0,1,sub2
	se_bc	1,2,sub2

	e_b	sub3
	e_bl	sub4
	e_bc	0,5,sub5
	e_bcl	1,10,sub5

	e_or2i 1, low@l
	e_or2i 2, high@h
	e_or2i 3, high_adjust@ha
	e_or2i 4, low_sdarel@sdarel@l
	e_or2i 5, high_sdarel@sdarel@h
	e_or2i 2, high_adjust_sdarel@sdarel@ha

	e_and2i. 1, low@l
	e_and2i. 2, high@h
	e_and2i. 3, high_adjust@ha
	e_and2i. 4, low_sdarel@sdarel@l
	e_and2i. 5, high_sdarel@sdarel@h
	e_and2i. 2, high_adjust_sdarel@sdarel@ha
	e_and2i. 2, high_adjust_sdarel@sdarel@ha

	e_or2is 1, low@l
	e_or2is 2, high@h
	e_or2is 3, high_adjust@ha
	e_or2is 4, low_sdarel@sdarel@l
	e_or2is 5, high_sdarel@sdarel@h
	e_or2is 2, high_adjust_sdarel@sdarel@ha

	e_lis 1, low@l
	e_lis 2, high@h
	e_lis 3, high_adjust@ha
	e_lis 4, low_sdarel@sdarel@l
	e_lis 5, high_sdarel@sdarel@h
	e_lis 2, high_adjust_sdarel@sdarel@ha

	e_and2is. 1, low@l
	e_and2is. 2, high@h
	e_and2is. 3, high_adjust@ha
	e_and2is. 4, low_sdarel@sdarel@l
	e_and2is. 5, high_sdarel@sdarel@h
	e_and2is. 2, high_adjust_sdarel@sdarel@ha

	e_cmp16i 1, low@l
	e_cmp16i 2, high@h
	e_cmp16i 3, high_adjust@ha
	e_cmp16i 4, low_sdarel@sdarel@l
	e_cmp16i 5, high_sdarel@sdarel@h
	e_cmp16i 2, high_adjust_sdarel@sdarel@ha

	e_cmpl16i 1, low@l
	e_cmpl16i 2, high@h
	e_cmpl16i 3, high_adjust@ha
	e_cmpl16i 4, low_sdarel@sdarel@l
	e_cmpl16i 5, high_sdarel@sdarel@h
	e_cmpl16i 2, high_adjust_sdarel@sdarel@ha

	e_cmph16i 1, low@l
	e_cmph16i 2, high@h
	e_cmph16i 3, high_adjust@ha
	e_cmph16i 4, low_sdarel@sdarel@l
	e_cmph16i 5, high_sdarel@sdarel@h
	e_cmph16i 2, high_adjust_sdarel@sdarel@ha

	e_cmphl16i 1, low@l
	e_cmphl16i 2, high@h
	e_cmphl16i 3, high_adjust@ha
	e_cmphl16i 4, low_sdarel@sdarel@l
	e_cmphl16i 5, high_sdarel@sdarel@h
	e_cmphl16i 2, high_adjust_sdarel@sdarel@ha

	e_add2i. 1, low@l
	e_add2i. 2, high@h
	e_add2i. 3, high_adjust@ha
	e_add2i. 4, low_sdarel@sdarel@l
	e_add2i. 5, high_sdarel@sdarel@h
	e_add2i. 2, high_adjust_sdarel@sdarel@ha

	e_add2is 1, low@l
	e_add2is 2, high@h
	e_add2is 3, high_adjust@ha
	e_add2is 4, low_sdarel@sdarel@l
	e_add2is 5, high_sdarel@sdarel@h
	e_add2is 2, high_adjust_sdarel@sdarel@ha

	e_mull2i 1, low@l
	e_mull2i 2, high@h
	e_mull2i 3, high_adjust@ha
	e_mull2i 4, low_sdarel@sdarel@l
	e_mull2i 5, high_sdarel@sdarel@h
	e_mull2i 2, high_adjust_sdarel@sdarel@ha
