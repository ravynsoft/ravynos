  .text

;;; Test some common instruction patterns for disabling/enabling interrupts.
;;; "MOV &FOO,r10" is used as an artbitrary statement which isn't a NOP, to
;;; break up the instructions being tested.

fn1:
;;; 1: Test EINT
;; 430 ISA: NOP *not* required before *or* after EINT
;; 430x ISA: NOP *is* required before *and* after EINT
  MOV &FOO,r10

  EINT

  MOV &FOO,r10

  BIS.W #8,SR		; Alias for EINT

  MOV &FOO,r10
;;; 2: Test DINT
;; 430 ISA: NOP *is* required after DINT
;; 430x ISA: NOP *is* required after DINT
  MOV &FOO,r10

  DINT
  NOP

  MOV &FOO,r10

  BIC.W #8,SR		; Alias for DINT
  NOP

  MOV &FOO,r10
;;; 3: Test EINT immediately before DINT
;; 430 ISA: NOP *not* required.
;; 430x ISA: NOP *is* required between EINT and DINT
  MOV &FOO,r10

  NOP
  EINT
  DINT
  NOP

  MOV &FOO,r10

  NOP
  BIS.W #8,SR		; Alias for EINT
  BIC.W #8,SR		; Alias for DINT
  NOP

  MOV &FOO,r10
;;; 4: Test DINT immediately before EINT
;; 430 ISA: NOP *is* required after DINT.
;; 430x ISA: NOP *is* required after DINT and before EINT. Ensure only one
;; warning is emitted.
  MOV &FOO,r10

  NOP
  DINT
  EINT
  NOP

  MOV &FOO,r10

  BIC.W #8,SR		; Alias for DINT
  BIS.W #8,SR		; Alias for EINT
  NOP

  MOV &FOO,r10

;;; 5: Test EINT last insn in file

  NOP
  EINT
