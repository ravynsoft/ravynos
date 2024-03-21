; Test-case primarily from PR26589, which could have been locref3.d.
; A local (like "static") function, with all ELF decorations.
.text
 .type	alocalfunc,@function
 .p2align 1
alocalfunc:
	nop
.Lfe1:
	.size	alocalfunc,.Lfe1-alocalfunc

; Random absolute reference to the address of alocalfunc, requiring a
; runtime relocation in code that needs to be PIC/PIE.
 .data
 .dword alocalfunc
