# Check if objdump works correctly when some bits in instruction
# has non-default value

	vrndscalesd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7} # with null RC
# vrndscalesd	{sae}, $123, %xmm4, %xmm5, %xmm6{%k7}	 # with not-null RC
	.insn EVEX.66.0f3a.W1 0x0b, $123, {ru-sae}, %xmm4, %xmm5, %xmm6{%k7}

	vpminud	%zmm4, %zmm5, %zmm6{%k7}	# with 11 EVEX.{B,R'}
	vpminud	%zmm12, %zmm5, %zmm22{%k7}	# with not-11 EVEX.{B,R'}
# vpminud	%zmm4, %zmm5, %zmm6{%k7}	# with set EVEX.b bit
	.insn EVEX.66.0F38.W0 0x3b, {rn-sae}, %zmm4, %zmm5, %zmm6{%k7}

	vpmovdb	%zmm6, 2032(%rdx)		# with unset EVEX.b bit
# vpmovdb	%zmm6, 2032(%rdx)		# with set EVEX.b bit - we should get (bad) operand
	.insn EVEX.f3.0f38.W0 0x31, %zmm6, 2032(%rdx){1to4}
