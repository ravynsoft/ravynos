	.text
	.global _start
	.global _mainCRTStartup
_start:
_mainCRTStartup:
	.byte 1

	.data
	.p2align 12
start:
	.dc.a	__image_base__
	.dc.a	start
	.dc.a	__section_alignment__
	.dc.a	__file_alignment__
	.dc.a	__major_os_version__
	.dc.a	__minor_os_version__
	.dc.a	__major_subsystem_version__
	.dc.a	__minor_subsystem_version__
	.dc.a	end
end:
