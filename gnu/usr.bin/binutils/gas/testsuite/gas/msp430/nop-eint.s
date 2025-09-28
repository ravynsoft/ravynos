	.text

;;; Test for NOP warnings when enabling interrupts, which only applies to 430x
;;; ISA.
;;; "MOV &FOO,r10" is used as an artbitrary statement which isn't a NOP, to
;;; break up the instructions being tested.
  EINT

  MOV &FOO,r10
;;; Check aliases for which the GIE bit (bit 3) of the SR can be set
;;; These should all cause warnings
	BIS.W #8,R2
	MOV &FOO,r10

	BIS.W #8,SR
	MOV &FOO,r10

	MOV.W #8,R2
	MOV &FOO,r10

	MOV #0xf,R2
	MOV &FOO,r10

	MOV #0xffff,R2
	MOV &FOO,r10

;;; The above hopefully covers the legitimate ways the SR might be set
;;; but there are other insns that can technically modify R2, but shouldn't be
;;; used.

;;; Verify EINT/DINT chained behaviour

  EINT
  DINT

	MOV &FOO,r10

  DINT
  EINT

	MOV &FOO,r10

;;; Test EINT at end of file
  EINT
