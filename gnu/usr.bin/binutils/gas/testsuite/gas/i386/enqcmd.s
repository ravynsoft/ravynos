# Check ENQCMD[S] 32-bit instructions

	.allow_index_reg
	.text
_start:
	enqcmd (%ecx),%eax
	enqcmd (%si),%ax
	enqcmds (%ecx),%eax
	enqcmds (%si),%ax
	enqcmd foo, %cx
	enqcmd 0x1234, %cx
	enqcmds foo, %cx
	enqcmds 0x1234, %cx

	.intel_syntax noprefix
	enqcmd eax,[ecx]
	enqcmd ax,[si]
	enqcmds eax,[ecx]
	enqcmds ax,[si]
	enqcmd cx,ds:foo
	enqcmd cx,ds:0x1234
	enqcmds cx,ds:foo
	enqcmds cx,ds:0x1234
