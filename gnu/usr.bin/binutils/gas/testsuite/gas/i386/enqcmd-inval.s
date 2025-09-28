# Check error for ENQCMD[S] 32-bit instructions

	.allow_index_reg
	.text
_start:
	enqcmd (%si),%eax
	enqcmd (%esi),%ax
	enqcmds (%si),%eax
	enqcmds (%esi),%ax

	.intel_syntax noprefix
	enqcmd eax,[si]
	enqcmd ax,[esi]
	enqcmds eax,[si]
	enqcmds ax,[esi]
