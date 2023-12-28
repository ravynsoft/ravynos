; Check that .dtpoffd trivially works, for external (though presumably
; module-local) and local symbols with offsets.

	.section .tdata,"awT",@progbits
	.type	x, @object
	.size	x, 4
x:
	.dword 0

	.text
start:
	.dword 0x73696854
	.dtpoffd extsym+42
	.dword 0x61207369
	.dtpoffd x+2
	.dword 0x55566699
