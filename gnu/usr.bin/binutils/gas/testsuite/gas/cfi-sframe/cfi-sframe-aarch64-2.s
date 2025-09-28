## ARMv8.3 addded support a new security feature named Pointer Authentication. The
## main idea behind this is to use the unused bits in the pointer values.
## Each pointer is patched with a PAC before writing to memory, and is verified
## before using it.
## When the pointers are mangled, the stack trace generator needs to know so it
## can mask off the PAC from the pointer value to recover the return address,
## and conversely, skip doing so if the pointers are not mangled.
##
## .cfi_negate_ra_state CFI directive is used to convey this information.
##
## SFrame has support for this. This testcase ensures that the directive
## is interpreted successfully.
	.cfi_startproc
	.long 0
	.cfi_def_cfa_offset 16
	.cfi_negate_ra_state
	.long 0
	.cfi_endproc
