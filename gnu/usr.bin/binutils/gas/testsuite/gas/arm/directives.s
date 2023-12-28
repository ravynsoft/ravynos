@ Check that various arm-specific directives can appear on the same line
	.cpu cortex-a5 ; .arch armv8-a ; .arch_extension fp ; .arch_extension nosimd ; .syntax unified ; .thumb; .code 16 ; movs r0, r1
	t1 .req r1 ; t2 .req r2 ;
	movs t1, t2 @ mov r1, r2

	.unreq t1;.unreq t2;
	t1 .req r2
	t2 .req r3
	movs t1, t2 @ movs r2, r3
	vmov.f32 s0, s1
	.eabi_attribute Tag_CPU_name, "custom_name" ; .eabi_attribute Tag_ABI_align8_preserved, 1;
