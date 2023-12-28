.text
.global foo_vpcs
.global foo_base
.global alias_vpcs
.global alias_base

.variant_pcs foo_vpcs
.variant_pcs alias_vpcs

foo_vpcs:
foo_base:
	bl foo_vpcs
	bl foo_base
	bl alias_vpcs
	bl alias_base

/* Check that the STO_AARCH64_VARIANT_PCS is not affected by .set.  */

.set alias_base, foo_vpcs
.set alias_vpcs, foo_base
