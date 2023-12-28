	.eqv	early_const,0x123456
	lui	$4,%lo(%neg(%lo(bar-foo)))
foo:
	nop
bar:
	lui	$4,%lo(%neg(%lo(bar-foo)))
	lui	$4,%hi(%gp_rel(early_const))
	lui	$4,%lo(%neg(%gp_rel(late_const)))
	.eqv	late_const,0x234567
