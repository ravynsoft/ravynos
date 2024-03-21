	# The priv instruction is explicit used, so we need to generate
	# the priv spec attributes even if the attributes are not set.
.ifdef priv_insn_a
	mret
.endif
.ifdef priv_insn_b
	sret
.endif
.ifdef priv_insn_c
	wfi
.endif
.ifdef priv_insn_d
	sfence.vma
.endif

	# Obselete priv instruction.
.ifdef priv_insn_e
	sfence.vm
.endif
