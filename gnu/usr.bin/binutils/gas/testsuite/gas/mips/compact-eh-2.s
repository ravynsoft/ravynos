	.gnu_attribute 4, 1
	.abicalls
        .hidden DW.ref.__gnu_compact_pr2
        .weak   DW.ref.__gnu_compact_pr2
        .section        .data.DW.ref.__gnu_compact_pr2,"awG",@progbits,DW.ref.__gnu_compact_pr2,comdat
        .align  2
        .type   DW.ref.__gnu_compact_pr2, @object
        .size   DW.ref.__gnu_compact_pr2, 4
DW.ref.__gnu_compact_pr2:
        .word   __gnu_compact_pr2
	.text
	.align	2
	.globl	_Z3fooi
	.cfi_sections .eh_frame_entry
$LFB0 = .
	.cfi_startproc
	.cfi_personality 0x1b, DW.ref.__gnu_compact_pr2
	.set	nomips16
	.set	nomicromips
	.ent	_Z3fooi
	.type	_Z3fooi, @function
_Z3fooi:
	nop
	.end	_Z3fooi
	.size	_Z3fooi, .-_Z3fooi
	.cfi_fde_data 0x4,0x40
	.cfi_endproc
	.globl	__gnu_compact_pr2
