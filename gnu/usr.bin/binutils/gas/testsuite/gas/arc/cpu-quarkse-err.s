;;; Check if .cpu em4_quarkse has code-density, spfp and dpfp ops.
; { dg-do assemble { target arc*-*-* } }
	.cpu	quarkse_em
	sub_s 	r15,r2,r15	; code-density op
	dmulh11	r1,r2,r3	; dpfp op
	fadd	r1,r2,r3	; spfp op
	dsp_fp_div r2,r2,r3	; QuarkSE-EM specific
