	.text
	.global main
main:
	movz x0,:abs_g0_nc:global_a
	movk x0,:abs_g1_nc:global_a
	movk x0,:abs_g2_nc:global_a
	movk x0,:abs_g3:global_a
