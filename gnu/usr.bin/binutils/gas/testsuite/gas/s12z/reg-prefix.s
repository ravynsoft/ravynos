;;; This test checks that when assembling with --mreg-prefix=%
;;; registers can be distinguished from symbols.
	nop
d0:
	st %d1, %d0
	st %d1, d0
