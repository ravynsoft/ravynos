	.allow_index_reg

.macro test_insn mnemonic
	\mnemonic	 %xmm12, %xmm4, %xmm2
	{evex} \mnemonic %xmm12, %xmm4, %xmm2
	{vex}  \mnemonic %xmm12, %xmm4, %xmm2
	{vex3} \mnemonic %xmm12, %xmm4, %xmm2
	{vex}  \mnemonic (%rcx), %xmm4, %xmm2
	{vex3} \mnemonic (%rcx), %xmm4, %xmm2
	\mnemonic	 %xmm22, %xmm4, %xmm2
.endm

	.text
_start:
	test_insn vpdpbusd
	test_insn vpdpwssd
	test_insn vpdpbusds
	test_insn vpdpwssds

	.arch .avx_vnni
	vpdpbusd	%xmm12, %xmm4, %xmm2
	{vex3} vpdpbusd 2032(%rcx), %xmm4, %xmm2
