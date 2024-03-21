        .section .text
sub1:
        se_blr

sub2:
        se_blr

	.section .text
vle_reloc:
	se_b	sub1
	se_bl	sub1
	se_bc	0,1,sub2
	se_bc	1,2,sub2

	e_b	sub3
	e_bl	sub4
	e_bc	0,5,sub5
	e_bcl	1,10,sub5
