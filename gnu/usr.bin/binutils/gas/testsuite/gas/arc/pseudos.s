# Test pseudo instructions generation.
	push	r0
	pop	r1
.L1:
	brgt	r0, r1, @.L1	; Encode as BRLT<.d> c,b,s9
	brgt	r0, -1, @.L1	; Encode as BRGE<.d> b,u6+1,s9
	brgt	r0, 0x3F, @.L1	; Encode as BRLT limm,b,s9
	brgt	r0, -2, @.L1	; Encode as BRLT limm,b,s9
	brgt	-2, r0, @.L1	; Encode as BRLT c,limm,s9
	brgt	-2, -1, @.L1	; Encode as BRGE limm,u6+1,s9
	brgt	-2, 0x3E, @.L1	; Encode as BRGE limm,u6+1,s9

	brhi 	r1, r1, @.L1	; BRHI<.d> b,c,s9 Encode as BRLO<.d> c,b,s9
	brhi	r1, -1, @.L1	; BRHI<.d> b,u6,s9 Encode as BRHS<.d> b,u6+1,s9
	brhi	r1, 0x3F, @.L1	; BRHI b,limm,s9 Encode as BRLO limm,b,s9
	brhi	r1, -2, @.L1	; BRHI b,limm,s9 Encode as BRLO limm,b,s9
	brhi	-2, r0, @.L1	; BRHI limm,c,s9 Encode as BRLO c,limm,s9
	brhi	-2, -1, @.L1	; BRHI limm,u6,s9 Encode as BRHS limm,u6+1,s9
	brhi	-2, 0x3E, @.L1	; BRHI limm,u6,s9 Encode as BRHS limm,u6+1,s9

	brle 	r1, r1, @.L1	; BRLE<.d> b,c,s9 Encode as BRGE<.d> c,b,s9
	brle	r1, -1, @.L1	; BRLE<.d> b,u6,s9 Encode as BRLT<.d> b,u6+1,s9
	brle	r1, 0x3F, @.L1	; BRLE b,limm,s9 Encode as BRGE limm,b,s9
	brle	r1, -2, @.L1	; BRLE b,limm,s9 Encode as BRGE limm,b,s9
	brle	-2, r0, @.L1	; BRLE limm,c,s9 Encode as BRGE c,limm,s9
	brle	-2, -1, @.L1	; BRLE limm,u6,s9 Encode as BRLT limm,u6+1,s9
	brle	-2, 0x3E, @.L1	; BRLE limm,u6,s9 Encode as BRLT limm,u6+1,s9

        brle    r1, r1, @.L1    ; BRLS<.d> b,c,s9 Encode as BRHS<.d> c,b,s9
        brle    r1, -1, @.L1    ; BRLS<.d> b,u6,s9 Encode as BRLO b,u6+1,s9
        brle    r1, 0x3F, @.L1  ; BRLS b,limm,s9 Encode as BRHS limm,b,s9
        brle    r1, -2, @.L1    ; BRLS limm,c,s9 Encode as BRHS c,limm,s9
        brle    -2, r0, @.L1    ; BRLS limm,c,s9 Encode as BRHS c,limm,s9
        brle    -2, -1, @.L1    ; BRLS limm,u6,s9 Encode as BRLO limm,u6+1,s9
        brle    -2, 0x3E, @.L1  ; BRLS limm,u6,s9 Encode as BRLO limm,u6+1,s9
