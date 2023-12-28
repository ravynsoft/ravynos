        # The execution order of the following DCTI couple is
        # unpredictable in arches prior to V9, and also in arches
        # starting with V9C.
        bcc	1f
        b	1f
        nop
1:
        # The following DCTI couple is deprecated starting with V9C
        # (UltraSPARC Architecture 2005) but it is ok in previous
        # arches.
	b	1b
        b	1b
        nop
