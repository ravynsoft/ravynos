#R_AARCH64_GOT_LD_PREL19
        .comm   sym312_notused,4
.text
_test_gc_rel312:
        nop
	ldr     x3, [x2, #:got_lo12:sym312_notused]

