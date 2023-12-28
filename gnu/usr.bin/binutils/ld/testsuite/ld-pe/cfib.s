 .cfi_sections	.debug_frame
 .section .text$abc,"x"
 .linkonce discard
 .align 2
 .globl _tst
 .def _tst; .scl 2; .type 32; .endef
_tst:
 .cfi_startproc
 .long 0
 .cfi_def_cfa_offset 16
 .cfi_offset 6, -16
 .long 1
 .cfi_def_cfa 7, 8
 .cfi_restore 6
 .long 2
 .cfi_endproc
