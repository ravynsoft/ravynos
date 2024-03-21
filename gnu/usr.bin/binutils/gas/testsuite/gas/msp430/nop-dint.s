	.text

;;; Test for NOP warnings when disabling interrupts, which are common to both
;;; 430 and 430x ISA.
;;; "MOV &FOO,r10" is used as an artbitrary statement which isn't a NOP, to
;;; break up the instructions being tested.

;;; Test NOP required after DINT
	DINT

	MOV &FOO,r10
;;; Check aliases for which the GIE bit (bit 3) of the SR can be cleared
;;; These should all cause warnings
	BIC.W #8,R2
	MOV &FOO,r10

	BIC.W #8,SR
	MOV &FOO,r10

	MOV.W #0,R2
	MOV &FOO,r10

	MOV.W #7,R2
	MOV &FOO,r10

	MOV.W #0xf007,R2
	MOV &FOO,r10

	CLR R2
	MOV &FOO,r10

;;; The above hopefully covers the legitimate ways the SR might be cleared,
;;; but there are other insns that can technically modify R2, but shouldn't be
;;; used.

;;; Test DINT at end of file
	DINT
