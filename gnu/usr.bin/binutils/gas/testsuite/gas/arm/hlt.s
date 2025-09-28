# Test that hlt is available for all architectures.
.macro gen_for_arch arch, has_thumb
	.arch \arch
	.ifc "yes","\has_thumb"
	.thumb
	hlt
	hlt 0xf
	.endif
	.arm
	hlt
	hlt 0xf
.endm

gen_for_arch armv8-a, yes
gen_for_arch armv7-a, yes
gen_for_arch armv6, yes
gen_for_arch armv5t, yes
gen_for_arch armv4t, yes
gen_for_arch armv3, no
gen_for_arch armv2, no
gen_for_arch armv1, no

