# Test the R_ARM_TARGET2 relocation
# Copied from ld/testsuite/ld-arm/arm-target2.s
	.text
	.global _start
_start:
	.word foo(target2)
	.word foo+0x1234(target2)
	.word foo+0xcdef0000(target2)
	.word foo+0x76543210(target2)
foo:
