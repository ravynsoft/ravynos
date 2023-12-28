	.file	"main.c"
  .section  .preinit_array,"aw"
  .balign 2
  .short  42
.text
	.balign 2
	.global	main
	.type	main, @function
main:
; start of function
; framesize_regs:     0
; framesize_locals:   0
; framesize_outgoing: 0
; framesize:          0
; elim ap -> fp       2
; elim fp -> sp       0
; saved regs:(none)
	; start of prologue
	; end of prologue
.L2:
	BR	#.L2
	.size	main, .-main
