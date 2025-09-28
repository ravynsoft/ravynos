       .text
        .globl  "_DllMainCRTStartup@12"
"_DllMainCRTStartup@12":
        .globl  _DllMainCRTStartup
_DllMainCRTStartup:
        .globl  DllMainCRTStartup
DllMainCRTStartup:
	nop

	.section .rdata,"dr"
_testval:
        .long   1
        .long   2
