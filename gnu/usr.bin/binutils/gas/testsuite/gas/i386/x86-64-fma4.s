# Check 64bit FMA4 instructions
	
	.allow_index_reg
	.text
_start:
	
	vfmaddpd %ymm4,%ymm6,%ymm2,%ymm7
	vfmaddpd (%rcx),%ymm6,%ymm2,%ymm7
	vfmaddps %ymm4,%ymm6,%ymm2,%ymm7
	vfmaddps (%rcx),%ymm6,%ymm2,%ymm7
	vfmaddps %xmm4,0x01(%rdx,%rbx,8),%xmm7,%xmm11
	vfmaddps %xmm8,0x80(%rcx,%rax,4),%xmm6,%xmm4
	vfmaddsubpd %ymm4,%ymm6,%ymm2,%ymm7
	vfmaddsubpd (%rcx),%ymm6,%ymm2,%ymm7
	vfmaddsubps %ymm4,%ymm6,%ymm2,%ymm7
	vfmaddsubps (%rcx),%ymm6,%ymm2,%ymm7
	vfmaddpd %xmm4,%xmm6,%xmm2,%xmm7
	vfmaddpd (%rcx),%xmm6,%xmm2,%xmm7
	vfmaddpd %xmm4,(%rcx),%xmm2,%xmm7
	vfmaddps %xmm4,%xmm6,%xmm2,%xmm7
	vfmaddps (%rcx),%xmm6,%xmm2,%xmm7
	vfmaddps %xmm4,(%rcx),%xmm2,%xmm7
	vfmaddsubpd %xmm4,%xmm6,%xmm2,%xmm7
	vfmaddsubpd (%rcx),%xmm6,%xmm2,%xmm7
	vfmaddsubpd %xmm4,(%rcx),%xmm2,%xmm7
	vfmaddsubps %xmm4,%xmm6,%xmm2,%xmm7
	vfmaddsubps (%rcx),%xmm6,%xmm2,%xmm7
	vfmaddsubps %xmm4,(%rcx),%xmm2,%xmm7
	vfmaddsd %xmm4,%xmm6,%xmm2,%xmm7
	vfmaddsd (%rcx),%xmm6,%xmm2,%xmm7
	vfmaddsd %xmm4,(%rcx),%xmm2,%xmm7
	vfmaddss %xmm4,%xmm6,%xmm2,%xmm7
	vfmaddss (%rcx),%xmm6,%xmm2,%xmm7
	vfmaddss %xmm4,(%rcx),%xmm2,%xmm7
	vfnmaddpd %ymm4,%ymm6,%ymm2,%ymm7
	vfnmaddpd (%rcx),%ymm6,%ymm2,%ymm7
	vfnmaddps %ymm4,%ymm6,%ymm2,%ymm7
	vfnmaddps (%rcx),%ymm6,%ymm2,%ymm7
	vfnmsubpd %ymm4,%ymm6,%ymm2,%ymm7
	vfnmsubpd (%rcx),%ymm6,%ymm2,%ymm7
	vfnmsubps %ymm4,%ymm6,%ymm2,%ymm7
	vfnmsubps (%rcx),%ymm6,%ymm2,%ymm7
	vfnmaddpd %xmm4,%xmm6,%xmm2,%xmm7
	vfnmaddpd (%rcx),%xmm6,%xmm2,%xmm7
	vfnmaddpd %xmm4,(%rcx),%xmm2,%xmm7
	vfnmaddps %xmm4,%xmm6,%xmm2,%xmm7
	vfnmaddps (%rcx),%xmm6,%xmm2,%xmm7
	vfnmaddps %xmm4,(%rcx),%xmm2,%xmm7
	vfnmsubpd %xmm4,%xmm6,%xmm2,%xmm7
	vfnmsubpd (%rcx),%xmm6,%xmm2,%xmm7
	vfnmsubpd %xmm4,(%rcx),%xmm2,%xmm7
	vfnmsubps %xmm4,%xmm6,%xmm2,%xmm7
	vfnmsubps (%rcx),%xmm6,%xmm2,%xmm7
	vfnmsubps %xmm4,(%rcx),%xmm2,%xmm7
	vfnmaddsd %xmm4,%xmm6,%xmm2,%xmm7
	vfnmaddsd (%rcx),%xmm6,%xmm2,%xmm7
	vfnmaddsd %xmm4,(%rcx),%xmm2,%xmm7
	vfnmsubsd %xmm4,%xmm6,%xmm2,%xmm7
	vfnmsubsd (%rcx),%xmm6,%xmm2,%xmm7
	vfnmsubsd %xmm4,(%rcx),%xmm2,%xmm7
	vfnmaddss %xmm4,%xmm6,%xmm2,%xmm7
	vfnmaddss (%rcx),%xmm6,%xmm2,%xmm7
	vfnmaddss %xmm4,(%rcx),%xmm2,%xmm7
	vfnmsubss %xmm4,%xmm6,%xmm2,%xmm7
	vfnmsubss (%rcx),%xmm6,%xmm2,%xmm7
	vfmaddpd (%r13,%rcx),%xmm11,%xmm3,%xmm4
	vfmaddpd 0xbe(%r9,%rax,8),%xmm9,%xmm1,%xmm7
	vfmsubpd (%r13,%rcx),%xmm11,%xmm3,%xmm4

