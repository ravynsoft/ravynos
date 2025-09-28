# PowerPC 750 paired single precision tests
	.text
start:
	psq_l	0, 4(3), 1, 5
	psq_lu	1, 8(2), 0, 3
	psq_lux	2, 5, 4, 1, 2
	psq_lx	3, 2, 4, 0, 5
	psq_st	3, 8(2), 0, 3
	psq_stu	3, 8(2), 0, 7
	psq_stux 2, 3, 4, 0, 5
	psq_stx	6, 7, 8, 1, 4
	ps_abs	5,7
	ps_abs.	5,7
	ps_add	1,2,3
	ps_add. 1,2,3
	ps_cmpo0 3,2,4
	ps_cmpo1 3,2,4
	ps_cmpu0 3,2,4
	ps_cmpu1 3,2,4
	ps_div 2,4,6
	ps_div. 2,4,6
	ps_madd 0,1,2,3
	ps_madd. 0,1,2,3
	ps_madds0 1,2,3,4
	ps_madds0. 1,2,3,4
	ps_madds1 1,2,3,4
	ps_madds1. 1,2,3,4
	ps_merge00 2,4,6
	ps_merge00. 2,4,6
	ps_merge01 2,4,6
	ps_merge01. 2,4,6
	ps_merge10 2,4,6
	ps_merge10. 2,4,6
	ps_merge11 2,4,6
	ps_merge11. 2,4,6
	ps_mr 3,5
	ps_mr. 3,5
	ps_msub 2,4,6,8
	ps_msub. 2,4,6,8
	ps_mul 2,3,5
	ps_mul. 2,3,5
	ps_muls0 3,4,7
	ps_muls0. 3,4,7
	ps_muls1 3,4,7
	ps_muls1. 3,4,7
	ps_nabs	1,5
	ps_nabs. 1,5
	ps_neg 1,5
	ps_neg. 1,5
	ps_nmadd 1,3,5,7
	ps_nmadd. 1,3,5,7
	ps_nmsub 1,3,5,7
	ps_nmsub. 1,3,5,7
	ps_res 9,3
	ps_res. 9,3
	ps_rsqrte 9,3
	ps_rsqrte. 9,3
	ps_sel 1,2,3,4
	ps_sel. 1,2,3,4
	ps_sub 5,11,2
	ps_sub. 5,11,2
	ps_sum0 2,5,9,10
	ps_sum0. 2,5,9,10
	ps_sum1 2,5,9,10
	ps_sum1. 2,5,9,10
	dcbz_l 3,5
