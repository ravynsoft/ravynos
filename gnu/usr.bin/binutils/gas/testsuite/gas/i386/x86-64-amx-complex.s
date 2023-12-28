# Check 64bit AMX-COMPLEX instructions

	.allow_index_reg
	.text
_start:
	tcmmimfp16ps	%tmm4, %tmm5, %tmm6	 #AMX-COMPLEX
	tcmmimfp16ps	%tmm1, %tmm2, %tmm3	 #AMX-COMPLEX
	tcmmrlfp16ps	%tmm4, %tmm5, %tmm6	 #AMX-COMPLEX
	tcmmrlfp16ps	%tmm1, %tmm2, %tmm3	 #AMX-COMPLEX

.intel_syntax noprefix
	tcmmimfp16ps	tmm6, tmm5, tmm4	 #AMX-COMPLEX
	tcmmimfp16ps	tmm3, tmm2, tmm1	 #AMX-COMPLEX
	tcmmrlfp16ps	tmm6, tmm5, tmm4	 #AMX-COMPLEX
	tcmmrlfp16ps	tmm3, tmm2, tmm1	 #AMX-COMPLEX
