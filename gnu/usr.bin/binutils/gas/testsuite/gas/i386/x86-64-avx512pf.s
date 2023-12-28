# Check 64bit AVX512PF instructions

	.allow_index_reg
	.text
_start:

	vgatherpf0dpd	123(%r14,%ymm31,8){%k1}	 # AVX512PF
	vgatherpf0dpd	123(%r14,%ymm31,8){%k1}	 # AVX512PF
	vgatherpf0dpd	256(%r9,%ymm31){%k1}	 # AVX512PF
	vgatherpf0dpd	1024(%rcx,%ymm31,4){%k1}	 # AVX512PF

	vgatherpf0dps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vgatherpf0dps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vgatherpf0dps	256(%r9,%zmm31){%k1}	 # AVX512PF
	vgatherpf0dps	1024(%rcx,%zmm31,4){%k1}	 # AVX512PF

	vgatherpf0qpd	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vgatherpf0qpd	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vgatherpf0qpd	256(%r9,%zmm31){%k1}	 # AVX512PF
	vgatherpf0qpd	1024(%rcx,%zmm31,4){%k1}	 # AVX512PF

	vgatherpf0qps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vgatherpf0qps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vgatherpf0qps	256(%r9,%zmm31){%k1}	 # AVX512PF
	vgatherpf0qps	1024(%rcx,%zmm31,4){%k1}	 # AVX512PF

	vgatherpf1dpd	123(%r14,%ymm31,8){%k1}	 # AVX512PF
	vgatherpf1dpd	123(%r14,%ymm31,8){%k1}	 # AVX512PF
	vgatherpf1dpd	256(%r9,%ymm31){%k1}	 # AVX512PF
	vgatherpf1dpd	1024(%rcx,%ymm31,4){%k1}	 # AVX512PF

	vgatherpf1dps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vgatherpf1dps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vgatherpf1dps	256(%r9,%zmm31){%k1}	 # AVX512PF
	vgatherpf1dps	1024(%rcx,%zmm31,4){%k1}	 # AVX512PF

	vgatherpf1qpd	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vgatherpf1qpd	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vgatherpf1qpd	256(%r9,%zmm31){%k1}	 # AVX512PF
	vgatherpf1qpd	1024(%rcx,%zmm31,4){%k1}	 # AVX512PF

	vgatherpf1qps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vgatherpf1qps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vgatherpf1qps	256(%r9,%zmm31){%k1}	 # AVX512PF
	vgatherpf1qps	1024(%rcx,%zmm31,4){%k1}	 # AVX512PF

	vscatterpf0dpd	123(%r14,%ymm31,8){%k1}	 # AVX512PF
	vscatterpf0dpd	123(%r14,%ymm31,8){%k1}	 # AVX512PF
	vscatterpf0dpd	256(%r9,%ymm31){%k1}	 # AVX512PF
	vscatterpf0dpd	1024(%rcx,%ymm31,4){%k1}	 # AVX512PF

	vscatterpf0dps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vscatterpf0dps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vscatterpf0dps	256(%r9,%zmm31){%k1}	 # AVX512PF
	vscatterpf0dps	1024(%rcx,%zmm31,4){%k1}	 # AVX512PF

	vscatterpf0qpd	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vscatterpf0qpd	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vscatterpf0qpd	256(%r9,%zmm31){%k1}	 # AVX512PF
	vscatterpf0qpd	1024(%rcx,%zmm31,4){%k1}	 # AVX512PF

	vscatterpf0qps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vscatterpf0qps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vscatterpf0qps	256(%r9,%zmm31){%k1}	 # AVX512PF
	vscatterpf0qps	1024(%rcx,%zmm31,4){%k1}	 # AVX512PF

	vscatterpf1dpd	123(%r14,%ymm31,8){%k1}	 # AVX512PF
	vscatterpf1dpd	123(%r14,%ymm31,8){%k1}	 # AVX512PF
	vscatterpf1dpd	256(%r9,%ymm31){%k1}	 # AVX512PF
	vscatterpf1dpd	1024(%rcx,%ymm31,4){%k1}	 # AVX512PF

	vscatterpf1dps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vscatterpf1dps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vscatterpf1dps	256(%r9,%zmm31){%k1}	 # AVX512PF
	vscatterpf1dps	1024(%rcx,%zmm31,4){%k1}	 # AVX512PF

	vscatterpf1qpd	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vscatterpf1qpd	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vscatterpf1qpd	256(%r9,%zmm31){%k1}	 # AVX512PF
	vscatterpf1qpd	1024(%rcx,%zmm31,4){%k1}	 # AVX512PF

	vscatterpf1qps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vscatterpf1qps	123(%r14,%zmm31,8){%k1}	 # AVX512PF
	vscatterpf1qps	256(%r9,%zmm31){%k1}	 # AVX512PF
	vscatterpf1qps	1024(%rcx,%zmm31,4){%k1}	 # AVX512PF

	.intel_syntax noprefix
	vgatherpf0dpd	[r14+ymm31*8-123]{k1}	 # AVX512PF
	vgatherpf0dpd	[r14+ymm31*8-123]{k1}	 # AVX512PF
	vgatherpf0dpd	[r9+ymm31+256]{k1}	 # AVX512PF
	vgatherpf0dpd	[rcx+ymm31*4+1024]{k1}	 # AVX512PF

	vgatherpf0dps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vgatherpf0dps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vgatherpf0dps	[r9+zmm31+256]{k1}	 # AVX512PF
	vgatherpf0dps	[rcx+zmm31*4+1024]{k1}	 # AVX512PF

	vgatherpf0qpd	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vgatherpf0qpd	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vgatherpf0qpd	[r9+zmm31+256]{k1}	 # AVX512PF
	vgatherpf0qpd	[rcx+zmm31*4+1024]{k1}	 # AVX512PF

	vgatherpf0qps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vgatherpf0qps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vgatherpf0qps	[r9+zmm31+256]{k1}	 # AVX512PF
	vgatherpf0qps	[rcx+zmm31*4+1024]{k1}	 # AVX512PF

	vgatherpf1dpd	[r14+ymm31*8-123]{k1}	 # AVX512PF
	vgatherpf1dpd	[r14+ymm31*8-123]{k1}	 # AVX512PF
	vgatherpf1dpd	[r9+ymm31+256]{k1}	 # AVX512PF
	vgatherpf1dpd	[rcx+ymm31*4+1024]{k1}	 # AVX512PF

	vgatherpf1dps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vgatherpf1dps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vgatherpf1dps	[r9+zmm31+256]{k1}	 # AVX512PF
	vgatherpf1dps	[rcx+zmm31*4+1024]{k1}	 # AVX512PF

	vgatherpf1qpd	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vgatherpf1qpd	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vgatherpf1qpd	[r9+zmm31+256]{k1}	 # AVX512PF
	vgatherpf1qpd	[rcx+zmm31*4+1024]{k1}	 # AVX512PF

	vgatherpf1qps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vgatherpf1qps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vgatherpf1qps	[r9+zmm31+256]{k1}	 # AVX512PF
	vgatherpf1qps	[rcx+zmm31*4+1024]{k1}	 # AVX512PF

	vscatterpf0dpd	[r14+ymm31*8-123]{k1}	 # AVX512PF
	vscatterpf0dpd	[r14+ymm31*8-123]{k1}	 # AVX512PF
	vscatterpf0dpd	[r9+ymm31+256]{k1}	 # AVX512PF
	vscatterpf0dpd	[rcx+ymm31*4+1024]{k1}	 # AVX512PF

	vscatterpf0dps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vscatterpf0dps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vscatterpf0dps	[r9+zmm31+256]{k1}	 # AVX512PF
	vscatterpf0dps	[rcx+zmm31*4+1024]{k1}	 # AVX512PF

	vscatterpf0qpd	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vscatterpf0qpd	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vscatterpf0qpd	[r9+zmm31+256]{k1}	 # AVX512PF
	vscatterpf0qpd	[rcx+zmm31*4+1024]{k1}	 # AVX512PF

	vscatterpf0qps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vscatterpf0qps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vscatterpf0qps	[r9+zmm31+256]{k1}	 # AVX512PF
	vscatterpf0qps	[rcx+zmm31*4+1024]{k1}	 # AVX512PF

	vscatterpf1dpd	[r14+ymm31*8-123]{k1}	 # AVX512PF
	vscatterpf1dpd	[r14+ymm31*8-123]{k1}	 # AVX512PF
	vscatterpf1dpd	[r9+ymm31+256]{k1}	 # AVX512PF
	vscatterpf1dpd	[rcx+ymm31*4+1024]{k1}	 # AVX512PF

	vscatterpf1dps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vscatterpf1dps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vscatterpf1dps	[r9+zmm31+256]{k1}	 # AVX512PF
	vscatterpf1dps	[rcx+zmm31*4+1024]{k1}	 # AVX512PF

	vscatterpf1qpd	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vscatterpf1qpd	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vscatterpf1qpd	[r9+zmm31+256]{k1}	 # AVX512PF
	vscatterpf1qpd	[rcx+zmm31*4+1024]{k1}	 # AVX512PF

	vscatterpf1qps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vscatterpf1qps	[r14+zmm31*8-123]{k1}	 # AVX512PF
	vscatterpf1qps	[r9+zmm31+256]{k1}	 # AVX512PF
	vscatterpf1qps	[rcx+zmm31*4+1024]{k1}	 # AVX512PF

