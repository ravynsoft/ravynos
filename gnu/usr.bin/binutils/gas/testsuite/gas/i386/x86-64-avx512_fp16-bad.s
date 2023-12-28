.text
	#vfcmaddcph %zmm30, %zmm29, %zmm30 dest and src registers must be distinct.
	.insn EVEX.f2.M6.W0 0x56, %zmm30, %zmm29, %zmm30

	#vfcmaddcph (%rcx), %zmm3, %zmm3 dest and src registers must be distinct.
	.insn EVEX.f2.M6.W0 0x56, (%rcx), %zmm3, %zmm3

	#vfcmaddcph %xmm3, %xmm2, %xmm2 dest and src registers must be distinct.
	.insn EVEX.f2.M6.W0 0x56, %xmm3, %xmm2, %xmm2

	#vfcmaddcsh %xmm3, %xmm2, %xmm3 dest and src registers must be distinct.
	.insn EVEX.LIG.f2.M6.W0 0x57, %xmm3, %xmm2, %xmm3

	#vfcmaddcsh %xmm3, %xmm2, %xmm2 dest and src registers must be distinct.
	.insn EVEX.LIG.f2.M6.W0 0x57, %xmm3, %xmm2, %xmm2
