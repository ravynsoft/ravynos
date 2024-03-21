@ This test ensures that the following attributes
@ are emitted in the proper order.
	.cpu arm7tdmi
	.eabi_attribute 63, "val"
	.eabi_attribute Tag_nodefaults, 0
	.eabi_attribute Tag_also_compatible_with, "\006\013"
	.eabi_attribute Tag_T2EE_use, 1
	.eabi_attribute Tag_conformance, "2.07"
	.eabi_attribute Tag_Virtualization_use, 1
