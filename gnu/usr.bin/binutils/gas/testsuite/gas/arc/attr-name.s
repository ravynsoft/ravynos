;;; Set the cpu to avoid the eventual warning when setting Tag_ARC_CPU_base.
	.cpu ARC600
	.arc_attribute Tag_ARC_PCS_config,1
	.arc_attribute Tag_ARC_CPU_base, 1
	.arc_attribute Tag_ARC_CPU_variation, 1
	.arc_attribute Tag_ARC_CPU_name, "random-cpu"
	.arc_attribute Tag_ARC_ABI_rf16, 1
	.arc_attribute Tag_ARC_ABI_osver, 3
	.arc_attribute Tag_ARC_ABI_sda, 2
	.arc_attribute Tag_ARC_ABI_pic, 2
	.arc_attribute Tag_ARC_ABI_tls, 25
	.arc_attribute Tag_ARC_ABI_enumsize, 1
	.arc_attribute Tag_ARC_ABI_exceptions, 1
	.arc_attribute Tag_ARC_ABI_double_size, 8
	.arc_attribute Tag_ARC_ISA_config, "CD,FPUDA"
	.arc_attribute Tag_ARC_ISA_apex, "QUARKSE"
	.arc_attribute Tag_ARC_ISA_mpy_option, 6
