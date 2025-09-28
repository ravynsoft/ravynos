	.arch armv8-a

	@ Tag_CPU_arch & Tag_CPU_arch_profile = v8 || v8-R
	.eabi_attribute Tag_CPU_arch, 14
	.eabi_attribute Tag_CPU_arch_profile, 'S'
