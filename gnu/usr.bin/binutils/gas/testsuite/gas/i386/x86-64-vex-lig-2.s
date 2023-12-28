# Check VEX non-LIG instructions with with -mavxscalar=256

	.allow_index_reg
	.text
_start:
	        vmovd	%eax, %xmm0
	        vmovd	(%rax), %xmm0
	{vex3}  vmovd	%eax, %xmm0
	{vex3}  vmovd	(%rax), %xmm0

	        vmovd	%xmm0, %eax
	        vmovd	%xmm0, (%rax)
	{vex3}  vmovd	%xmm0, %eax
	{vex3}  vmovd	%xmm0, (%rax)

	        vmovq	%xmm0, %xmm0
	        vmovq	(%rax), %xmm0
	{vex3}  vmovq	%xmm0, %xmm0
	{vex3}  vmovq	(%rax), %xmm0

	{store} vmovq	%xmm0, %xmm0
	        vmovq	%xmm0, (%rax)
	{vex3} {store} vmovq %xmm0, %xmm0
	{vex3}  vmovq	%xmm0, (%rax)

	        vextractps $0, %xmm0, %eax
	        vextractps $0, %xmm0, (%rax)

	        vpextrb $0, %xmm0, %eax
	        vpextrb $0, %xmm0, (%rax)

	        vpextrw $0, %xmm0, %eax
	{vex3}  vpextrw $0, %xmm0, %eax
	{store} vpextrw $0, %xmm0, %eax
	        vpextrw $0, %xmm0, (%rax)

	        vpextrd $0, %xmm0, %eax
	        vpextrd $0, %xmm0, (%rax)

	        vpextrq $0, %xmm0, %rax
	        vpextrq $0, %xmm0, (%rax)

	        vinsertps $0, %xmm0, %xmm0, %xmm0
	        vinsertps $0, (%rax), %xmm0, %xmm0

	        vpinsrb $0, %eax, %xmm0, %xmm0
	        vpinsrb $0, (%rax), %xmm0, %xmm0

	        vpinsrw $0, %eax, %xmm0, %xmm0
	        vpinsrw $0, (%rax), %xmm0, %xmm0
	{vex3}  vpinsrw $0, %eax, %xmm0, %xmm0
	{vex3}  vpinsrw $0, (%rax), %xmm0, %xmm0

	        vpinsrd $0, %eax, %xmm0, %xmm0
	        vpinsrd $0, (%rax), %xmm0, %xmm0

	        vpinsrq $0, %rax, %xmm0, %xmm0
	        vpinsrq $0, (%rax), %xmm0, %xmm0

	        vldmxcsr (%rax)
	        vstmxcsr (%rax)
	{vex3}  vldmxcsr (%rax)
	{vex3}  vstmxcsr (%rax)

	andn	(%rax), %eax, %eax
	bextr	%eax, (%rax), %eax
	blsi	(%rax), %eax
	blsmsk	(%rax), %eax
	blsr	(%rax), %eax

	bzhi	%eax, (%rax), %eax
	mulx	(%rax), %eax, %eax
	pdep	(%rax), %eax, %eax
	pext	(%rax), %eax, %eax
	rorx	$0, (%rax), %eax
	sarx	%eax, (%rax), %eax
	shlx	%eax, (%rax), %eax
	shrx	%eax, (%rax), %eax

	bextr	$0, (%rax), %eax
	blcfill	(%rax), %eax
	blci	(%rax), %eax
	blcic	(%rax), %eax
	blcmsk	(%rax), %eax
	blcs	(%rax), %eax
	blsfill	(%rax), %eax
	blsic	(%rax), %eax
	t1mskc	(%rax), %eax
	tzmsk	(%rax), %eax
