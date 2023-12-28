#source: tls-desc-ie.s
#target: [check_shared_lib_support]
#ld: -shared -z pac-plt -T bti-plt.ld
#objdump: -dr -j .plt
#...
Disassembly of section .plt:

.* \<.plt\>:
.*:	a9bf7bf0 	stp	x16, x30, \[sp, #-16\]!
.*:	90000090 	adrp	x16, 28000 <_GLOBAL_OFFSET_TABLE_>
.*:	f9401a11 	ldr	x17, \[x16, #48\]
.*:	9100c210 	add	x16, x16, #0x30
.*:	d61f0220 	br	x17
.*:	d503201f 	nop
.*:	d503201f 	nop
.*:	d503201f 	nop


.* \<__tls_get_addr@plt\>:
.*:	90000090 	adrp	x16, 28000 <_GLOBAL_OFFSET_TABLE_>
.*:	f9401e11 	ldr	x17, \[x16, #56\]
.*:	9100e210 	add	x16, x16, #0x38
.*:	d503219f 	autia1716
.*:	d61f0220 	br	x17
.*:	d503201f 	nop
