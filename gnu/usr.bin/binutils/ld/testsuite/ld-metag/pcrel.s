	.text
	.global _start
_start:	
	CALLR    D0.0, external
	CALLR    D0.0, global
	CALLR    D0.0, local

	.global global
global:
	nop
local:
	nop
