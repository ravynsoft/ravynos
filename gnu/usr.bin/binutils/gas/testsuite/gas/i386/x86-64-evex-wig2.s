# Check non-WIG EVEX instructions with -mevexwig=1

	.allow_index_reg
	.text
_start:
	vcvtsi2ss %eax, {rd-sae}, %xmm25, %xmm6
	vcvtsi2ss %eax, %xmm25, %xmm6
	vcvtsi2sd %eax, %xmm25, %xmm6
	vcvtusi2ss %eax, {rd-sae}, %xmm25, %xmm6
	vcvtusi2ss %eax, %xmm25, %xmm6
	vcvtusi2sd %eax, %xmm15, %xmm6
