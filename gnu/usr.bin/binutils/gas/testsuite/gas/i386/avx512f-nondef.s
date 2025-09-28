# Check if objdump works correctly when some bits in instruction
# has non-default value

	 vrndscalesd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7} # with null RC
# vrndscalesd	{sae}, $123, %xmm4, %xmm5, %xmm6{%k7}	 # with not-null RC
	.insn EVEX.66.0f3a.W1 0x0b, $123, {ru-sae}, %xmm4, %xmm5, %xmm6{%k7}

	 vpminud	%zmm4, %zmm5, %zmm6{%k7}	# with 11 EVEX.{B,R'}
# vpminud	%zmm4, %zmm5, %zmm6{%k7}	# with not-11 EVEX.{B,R'}
.byte 0x62, 0xc2, 0x55, 0x4f, 0x3b, 0xf4
# vpminud	%zmm4, %zmm5, %zmm6{%k7}	# with set EVEX.b bit
	.insn EVEX.66.0F38.W0 0x3b, {rn-sae}, %zmm4, %zmm5, %zmm6{%k7}

	 vpmovdb	%zmm6, 2032(%edx)	# with unset EVEX.b bit
# vpmovdb	%zmm6, 2032(%edx)		# with set EVEX.b bit - we should get (bad) operand
	.insn EVEX.f3.0f38.W0 0x31, %zmm6, 2032(%edx){1to4}

# vaddps xmm0, xmm0, xmm3 # with EVEX.z set
.byte 0x62, 0xf1, 0x7c, 0x88, 0x58, 0xc3

# vgatherdps (%ecx), %zmm0{%k7}			# without SIB / index register
	.insn EVEX.66.0F38.W0 0x92, (%ecx), %zmm0{%k7}
# vgatherdps (%bx,%xmm?), %zmm0{%k7}		# with 16-bit addressing
	.insn EVEX.66.0F38.W0 0x92, (%bx,%di), %zmm0{%k7}
# vgatherdps (%eax,%zmm1), %zmm0{%k7}{z}	# with set EVEX.z
	.insn EVEX.66.0F38.W0 0x92, (%eax,%zmm1), %zmm0{%k7}{z}
# vgatherdps (%eax,%zmm1), %zmm0		# without actual mask register
	.insn EVEX.66.0F38.W0 0x92, (%eax,%zmm1), %zmm0
