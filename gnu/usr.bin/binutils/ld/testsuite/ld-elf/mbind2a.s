/* Place bss_in_mbind0 in .mbind.bss section with sh_info == 0.  */
	.globl bss_in_mbind0
	.section .mbind.bss,"adw",%nobits,0
	.type bss_in_mbind0, %object
	.size bss_in_mbind0, 1
bss_in_mbind0:
	.zero 1

/* Place data_in_mbind3 in .mbind.data section with sh_info == 3.  */
	.globl data_in_mbind3
	.section .mbind.data,"adw",%progbits,0x3
	.type data_in_mbind3, %object
	.size data_in_mbind3, 1
data_in_mbind3:
	.byte 0
