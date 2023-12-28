# Check 32bit AVX512PF instructions

	.allow_index_reg
	.text
_start:

	vgatherpf0dpd	123(%ebp,%ymm7,8){%k1}	 # AVX512PF
	vgatherpf0dpd	123(%ebp,%ymm7,8){%k1}	 # AVX512PF
	vgatherpf0dpd	256(%eax,%ymm7){%k1}	 # AVX512PF
	vgatherpf0dpd	1024(%ecx,%ymm7,4){%k1}	 # AVX512PF

	vgatherpf0dps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vgatherpf0dps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vgatherpf0dps	256(%eax,%zmm7){%k1}	 # AVX512PF
	vgatherpf0dps	1024(%ecx,%zmm7,4){%k1}	 # AVX512PF

	vgatherpf0qpd	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vgatherpf0qpd	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vgatherpf0qpd	256(%eax,%zmm7){%k1}	 # AVX512PF
	vgatherpf0qpd	1024(%ecx,%zmm7,4){%k1}	 # AVX512PF

	vgatherpf0qps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vgatherpf0qps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vgatherpf0qps	256(%eax,%zmm7){%k1}	 # AVX512PF
	vgatherpf0qps	1024(%ecx,%zmm7,4){%k1}	 # AVX512PF

	vgatherpf1dpd	123(%ebp,%ymm7,8){%k1}	 # AVX512PF
	vgatherpf1dpd	123(%ebp,%ymm7,8){%k1}	 # AVX512PF
	vgatherpf1dpd	256(%eax,%ymm7){%k1}	 # AVX512PF
	vgatherpf1dpd	1024(%ecx,%ymm7,4){%k1}	 # AVX512PF

	vgatherpf1dps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vgatherpf1dps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vgatherpf1dps	256(%eax,%zmm7){%k1}	 # AVX512PF
	vgatherpf1dps	1024(%ecx,%zmm7,4){%k1}	 # AVX512PF

	vgatherpf1qpd	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vgatherpf1qpd	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vgatherpf1qpd	256(%eax,%zmm7){%k1}	 # AVX512PF
	vgatherpf1qpd	1024(%ecx,%zmm7,4){%k1}	 # AVX512PF

	vgatherpf1qps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vgatherpf1qps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vgatherpf1qps	256(%eax,%zmm7){%k1}	 # AVX512PF
	vgatherpf1qps	1024(%ecx,%zmm7,4){%k1}	 # AVX512PF

	vscatterpf0dpd	123(%ebp,%ymm7,8){%k1}	 # AVX512PF
	vscatterpf0dpd	123(%ebp,%ymm7,8){%k1}	 # AVX512PF
	vscatterpf0dpd	256(%eax,%ymm7){%k1}	 # AVX512PF
	vscatterpf0dpd	1024(%ecx,%ymm7,4){%k1}	 # AVX512PF

	vscatterpf0dps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vscatterpf0dps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vscatterpf0dps	256(%eax,%zmm7){%k1}	 # AVX512PF
	vscatterpf0dps	1024(%ecx,%zmm7,4){%k1}	 # AVX512PF

	vscatterpf0qpd	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vscatterpf0qpd	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vscatterpf0qpd	256(%eax,%zmm7){%k1}	 # AVX512PF
	vscatterpf0qpd	1024(%ecx,%zmm7,4){%k1}	 # AVX512PF

	vscatterpf0qps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vscatterpf0qps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vscatterpf0qps	256(%eax,%zmm7){%k1}	 # AVX512PF
	vscatterpf0qps	1024(%ecx,%zmm7,4){%k1}	 # AVX512PF

	vscatterpf1dpd	123(%ebp,%ymm7,8){%k1}	 # AVX512PF
	vscatterpf1dpd	123(%ebp,%ymm7,8){%k1}	 # AVX512PF
	vscatterpf1dpd	256(%eax,%ymm7){%k1}	 # AVX512PF
	vscatterpf1dpd	1024(%ecx,%ymm7,4){%k1}	 # AVX512PF

	vscatterpf1dps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vscatterpf1dps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vscatterpf1dps	256(%eax,%zmm7){%k1}	 # AVX512PF
	vscatterpf1dps	1024(%ecx,%zmm7,4){%k1}	 # AVX512PF

	vscatterpf1qpd	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vscatterpf1qpd	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vscatterpf1qpd	256(%eax,%zmm7){%k1}	 # AVX512PF
	vscatterpf1qpd	1024(%ecx,%zmm7,4){%k1}	 # AVX512PF

	vscatterpf1qps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vscatterpf1qps	123(%ebp,%zmm7,8){%k1}	 # AVX512PF
	vscatterpf1qps	256(%eax,%zmm7){%k1}	 # AVX512PF
	vscatterpf1qps	1024(%ecx,%zmm7,4){%k1}	 # AVX512PF

	.intel_syntax noprefix
	vgatherpf0dpd	[ebp+ymm7*8-123]{k1}	 # AVX512PF
	vgatherpf0dpd	[ebp+ymm7*8-123]{k1}	 # AVX512PF
	vgatherpf0dpd	[eax+ymm7+256]{k1}	 # AVX512PF
	vgatherpf0dpd	[ecx+ymm7*4+1024]{k1}	 # AVX512PF

	vgatherpf0dps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vgatherpf0dps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vgatherpf0dps	[eax+zmm7+256]{k1}	 # AVX512PF
	vgatherpf0dps	[ecx+zmm7*4+1024]{k1}	 # AVX512PF

	vgatherpf0qpd	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vgatherpf0qpd	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vgatherpf0qpd	[eax+zmm7+256]{k1}	 # AVX512PF
	vgatherpf0qpd	[ecx+zmm7*4+1024]{k1}	 # AVX512PF

	vgatherpf0qps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vgatherpf0qps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vgatherpf0qps	[eax+zmm7+256]{k1}	 # AVX512PF
	vgatherpf0qps	[ecx+zmm7*4+1024]{k1}	 # AVX512PF

	vgatherpf1dpd	[ebp+ymm7*8-123]{k1}	 # AVX512PF
	vgatherpf1dpd	[ebp+ymm7*8-123]{k1}	 # AVX512PF
	vgatherpf1dpd	[eax+ymm7+256]{k1}	 # AVX512PF
	vgatherpf1dpd	[ecx+ymm7*4+1024]{k1}	 # AVX512PF

	vgatherpf1dps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vgatherpf1dps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vgatherpf1dps	[eax+zmm7+256]{k1}	 # AVX512PF
	vgatherpf1dps	[ecx+zmm7*4+1024]{k1}	 # AVX512PF

	vgatherpf1qpd	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vgatherpf1qpd	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vgatherpf1qpd	[eax+zmm7+256]{k1}	 # AVX512PF
	vgatherpf1qpd	[ecx+zmm7*4+1024]{k1}	 # AVX512PF

	vgatherpf1qps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vgatherpf1qps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vgatherpf1qps	[eax+zmm7+256]{k1}	 # AVX512PF
	vgatherpf1qps	[ecx+zmm7*4+1024]{k1}	 # AVX512PF

	vscatterpf0dpd	[ebp+ymm7*8-123]{k1}	 # AVX512PF
	vscatterpf0dpd	[ebp+ymm7*8-123]{k1}	 # AVX512PF
	vscatterpf0dpd	[eax+ymm7+256]{k1}	 # AVX512PF
	vscatterpf0dpd	[ecx+ymm7*4+1024]{k1}	 # AVX512PF

	vscatterpf0dps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vscatterpf0dps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vscatterpf0dps	[eax+zmm7+256]{k1}	 # AVX512PF
	vscatterpf0dps	[ecx+zmm7*4+1024]{k1}	 # AVX512PF

	vscatterpf0qpd	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vscatterpf0qpd	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vscatterpf0qpd	[eax+zmm7+256]{k1}	 # AVX512PF
	vscatterpf0qpd	[ecx+zmm7*4+1024]{k1}	 # AVX512PF

	vscatterpf0qps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vscatterpf0qps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vscatterpf0qps	[eax+zmm7+256]{k1}	 # AVX512PF
	vscatterpf0qps	[ecx+zmm7*4+1024]{k1}	 # AVX512PF

	vscatterpf1dpd	[ebp+ymm7*8-123]{k1}	 # AVX512PF
	vscatterpf1dpd	[ebp+ymm7*8-123]{k1}	 # AVX512PF
	vscatterpf1dpd	[eax+ymm7+256]{k1}	 # AVX512PF
	vscatterpf1dpd	[ecx+ymm7*4+1024]{k1}	 # AVX512PF

	vscatterpf1dps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vscatterpf1dps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vscatterpf1dps	[eax+zmm7+256]{k1}	 # AVX512PF
	vscatterpf1dps	[ecx+zmm7*4+1024]{k1}	 # AVX512PF

	vscatterpf1qpd	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vscatterpf1qpd	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vscatterpf1qpd	[eax+zmm7+256]{k1}	 # AVX512PF
	vscatterpf1qpd	[ecx+zmm7*4+1024]{k1}	 # AVX512PF

	vscatterpf1qps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vscatterpf1qps	[ebp+zmm7*8-123]{k1}	 # AVX512PF
	vscatterpf1qps	[eax+zmm7+256]{k1}	 # AVX512PF
	vscatterpf1qps	[ecx+zmm7*4+1024]{k1}	 # AVX512PF

