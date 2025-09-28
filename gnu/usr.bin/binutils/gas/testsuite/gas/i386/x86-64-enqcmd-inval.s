# Check error for ENQCMD[S] 64-bit instructions

	.allow_index_reg
	.text
_start:
	enqcmd (%esi),%rax
	enqcmd (%eip),%rax
	enqcmd (%rsi),%eax
	enqcmd (%rip),%eax
	enqcmds (%esi),%rax
	enqcmds (%eip),%rax
	enqcmds (%rsi),%eax
	enqcmds (%rip),%eax

	.intel_syntax noprefix
	enqcmd rax,[esi]
	enqcmd eax,[rsi]
	enqcmds rax,[esi]
	enqcmds eax,[rsi]
