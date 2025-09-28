# Test the R_ARM_TARGET1 relocation
# Copied from ld/testsuite/ld-arm/arm-target1.s
	.text
	.global _start
_start:
	.word foo(target1)
foo:
